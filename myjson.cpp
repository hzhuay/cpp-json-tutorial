#include "myjson.h"
#include <limits>
#include <cstdio>
#include <cmath>
#include <memory>

// JsonValue的各种派生类和模板特化
namespace myjson {

using std::make_shared;
using std::move;
// using std::shared_ptr;
using std::string;

struct NullStruct {
    bool operator==(NullStruct) const { return true; }
    bool operator<(NullStruct) const { return false; }
};

template <Json::Type t, typename T>
class Value : public JsonValue {
protected:
    const T value;

    explicit Value(const T& v) : value(v) {}
    explicit Value(T&& v) : value(move(v)) {}

    Json::Type type() const override { return t; }
};

class JsonNull final : public Value<Json::JSON_NULL, NullStruct> {
public:
    explicit JsonNull() : Value({}) {}
};

class JsonBool final : public Value<Json::JSON_BOOL, bool> {
    bool bool_value() const { return value; }

public:
    explicit JsonBool(bool v) : Value(v) {}
};

class JsonInt final : public Value<Json::JSON_NUMBER, int> {
    double number_value() const { return value; }
    int int_value() const { return value; }

public:
   explicit JsonInt(int v) : Value(v) {}
};

class JsonDouble final : public Value<Json::JSON_NUMBER, double> {
    double number_value() const { return value; }
    // 使用static_cast安全转型
    int int_value() const { return static_cast<int>(value); }

public:
    JsonDouble(double v) : Value(v) {}
};

class JsonString final : public Value<Json::JSON_STRING, string> {
    const string& string_value() const { return value; }
public:
    JsonString(const string &v) : Value(v) {}
    JsonString(string &&v): Value(move(v)) {}
};

class JsonArray final : public Value<Json::JSON_ARRAY, Json::array> {
    const Json::array& array_value() const override { return value; }
    const Json& operator[] (size_t i) const override;
public:
     JsonArray(const Json::array& v): Value(v) {}
     JsonArray(Json::array&& v): Value(move(v)) {}
};

class JsonObject final : public Value<Json::JSON_OBJECT, Json::object> {
    const Json::object& object_value() const override { return value; }
    const Json& operator[] (const string& key) const override;
public:
    JsonObject(const Json::object& v): Value(v) {}
    JsonObject(Json::object&& v): Value(move(v)) {}
};

/*
    使用单例模式，创建一系列可以公用的静态值和动态值的初始值。
*/
struct Singleton {
    const std::shared_ptr<JsonValue> null = make_shared<JsonNull>();
    const std::shared_ptr<JsonValue> t = make_shared<JsonBool>(true);
    const std::shared_ptr<JsonValue> f = make_shared<JsonBool>(false);
    const string empty_string;
    const Json::array empty_array;
    const Json::object empty_object;
    // const Json empty_json;
    Singleton() {}
};

// 利用静态局部变量，实现单例
static const Singleton& singleton() {
    static const Singleton s;
    return s;
}

static const Json& static_null() {
    static const Json json_null;
    return json_null;
}

// JsonValue的默认值返回函数
// 这里留一个问题，这样的设计是每种类型的值都有对应的取值函数，特化子类用对应的取值方法覆盖默认方法。
// 为什么不设计为统一接口，根据模板来推导返回值类型呢？
bool                JsonValue::bool_value()                     const { return false; }
double              JsonValue::number_value()                   const { return 0; }
int                 JsonValue::int_value()                      const { return 0; }
const string&       JsonValue::string_value()                   const { return singleton().empty_string; }
const Json::array&  JsonValue::array_value()                    const { return singleton().empty_array; }
const Json::object& JsonValue::object_value()                   const { return singleton().empty_object; }
const Json& JsonValue::operator[](size_t i) const {
    return static_null();
}
const Json& JsonValue::operator[](const string& key) const {
    return static_null();
}
/*
    Json
*/

// 获取Json的类型
Json::Type Json::type() const { return v_ptr->type(); }

// Json的一系列构造函数
Json::Json() noexcept:                  v_ptr(singleton().null) {}
Json::Json(State s) :                   v_ptr(singleton().null), state(s) {}
Json::Json(bool value) :                v_ptr(make_shared<JsonBool>(value)) {}
Json::Json(int value) :                 v_ptr(make_shared<JsonInt>(value)) {}
Json::Json(double value) :              v_ptr(make_shared<JsonDouble>(value)) {}
Json::Json(const string& value):        v_ptr(make_shared<JsonString>(value)) {}
Json::Json(string&& value):             v_ptr(make_shared<JsonString>(move(value))) {}
Json::Json(const char* value):          v_ptr(make_shared<JsonString>(value)) {}
Json::Json(const Json::array& value):   v_ptr(make_shared<JsonArray>(value)) {}
Json::Json(Json::array&& value):        v_ptr(make_shared<JsonArray>(move(value))) {}
Json::Json(const Json::object& value):  v_ptr(make_shared<JsonObject>(value)) {}
Json::Json(Json::object&& value):       v_ptr(make_shared<JsonObject>(move(value))) {}

// Json的访问器
bool Json::bool_value()                             const { return v_ptr->bool_value(); }
int Json::int_value()                               const { return v_ptr->int_value(); }
double Json::number_value()                         const { return v_ptr->number_value(); }
const string& Json::string_value()                  const { return v_ptr->string_value(); }
const Json::array& Json::array_value()              const { return v_ptr->array_value(); }
const Json::object& Json::object_value()            const { return v_ptr->object_value(); }
const Json& Json::operator[] (size_t i)             const { return (*v_ptr)[i]; }
const Json& Json::operator[] (const string& key)    const { return (*v_ptr)[key]; }

const Json& JsonArray::operator[] (size_t i) const {
    if (i >= value.size())
        return static_null();
    else return value[i];
}

const Json& JsonObject::operator[] (const string& key) const {
    auto iter = value.find(key);
    return (iter == value.end()) ? static_null() : iter->second;
}

/*
    JsonParser
*/

// JsonParser
Json JsonParser::parse_json() {
    char ch = get_next_token();
    switch (ch) {
        case 'n': return parse_literal("null", Json());
        case 't': return parse_literal("true", Json(true));
        case 'f': return parse_literal("false", Json(false));
        case '"': return parse_string();
        case '[': return parse_array();
        case '{': return parse_object();
        case '\0': return Json(JSON_PARSE_EXPECT_VALUE);
        default: i--; return parse_number();
    }
}

// 解析空白字符
void JsonParser::parse_whitespace() {
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
        i++;
}

// 增加i使跳过空白字符，获得下一个token的起始字符
char JsonParser::get_next_token() {
    parse_whitespace();
    if (i == str.size())
        return fail('\0');
    return str[i++];
}

// 解析null, bool这样的字面量
Json JsonParser::parse_literal(const string& expected, Json res) {
    assert(i != 0);
    i--;
    if (str.compare(i, expected.length(), expected) == 0) {
        i += expected.length();
        return res;
    } else {
        return fail(Json(JSON_PARSE_INVALID_VALUE));
    }
}

static inline bool in_range(char x, char lower, char upper) {
    return (x >= lower && x <= upper);
}

// 利用std::strtod来将字符串解析为number
/*
number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit
*/
Json JsonParser::parse_number() {
    size_t start = i;  //记录起始位置

    // 可选的负号部分
    if (str[i] == '-') i++;  //跳过负号

    // int部分
    if (str[i] == '0') i++;
    else {
        if (!in_range(str[i], '1', '9'))
            return fail(Json(JSON_PARSE_INVALID_VALUE));  //检查至少有一个digit
        for (i++; in_range(str[i], '0', '9'); i++);  //跳过所有digit
    }

    if (str[i] != '.' && str[i] != 'e' && str[i] != 'E' && (i - start) <=
            static_cast<size_t>(std::numeric_limits<int>::digits10)) {
        // std::numeric_limits<int>::digits10表示用10进制表示int的最大值需要的位数
        // 将该字符串转化为int
        return std::atoi(str.c_str() + start);
    }

    // Decimal部分，从这开始使用double
    if (str[i] == '.') {
        i++;
        if (!in_range(str[i], '0', '9'))  //检查至少有一个digit
            return fail(Json(JSON_PARSE_INVALID_VALUE));
        for (i++; in_range(str[i], '0', '9'); i++);
    }

    // Exponent部分
    if (str[i] == 'e' || str[i] == 'E') {
        i++;
        if (str[i] == '+' || str[i] == '-') i++;    //跳过正负号
        if (!in_range(str[i], '0', '9'))
            return fail(Json(JSON_PARSE_INVALID_VALUE));
        for (i++; in_range(str[i], '0', '9'); i++);
    }
    return std::strtod(str.c_str() + start, nullptr);
}


// 在leptjson中这个方法是手动计算的。也可以采用标准库的strtol，但是这个计算足够简单，无法一眼判断是否有替换的必要。
bool JsonParser::parse_hex4(unsigned int& u) {
    for (size_t j = 0; j < 4; j++) {
        char ch = str[i++];
        u <<= 4;
        if      (in_range(ch, '0', '9')) u |= ch - '0';
        else if (in_range(ch, 'A', 'F')) u |= ch - 'A' + 10;
        else if (in_range(ch, 'a', 'f')) u |= ch - 'a' + 10;
        else return false;
    }
    return true;
}

void JsonParser::encode_utf8(unsigned int u, string& out) {
    if (u <= 0x7F) {
        out += static_cast<char>(u);
    } else if (u <= 0x7FF) {
        out += static_cast<char>(0xC0 | (u >> 6));
        out += static_cast<char>(0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (u >> 12));
        out += static_cast<char>(0x80 | ((u >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        out += static_cast<char>(0xF0 | (u >> 18));
        out += static_cast<char>(0x80 | ((u >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((u >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (u & 0x3F));
    }
}

// 解析字符串
string JsonParser::parse_string() {
    string out;
    while (i < str.size()) {
        char ch = str[i++];
        switch (ch) {
            case '"':
                return out;
            case '\\':
                switch (str[i++]) {
                    case '\\':  out += '\\'; break;
                    case 'b':   out += '\b'; break;
                    case 'f':   out += '\f'; break;
                    case 'n':   out += '\n'; break;
                    case 'r':   out += '\r'; break;
                    case 't':   out += '\t'; break;
                    case '"':   out += '\"'; break;
                    case '/':   out += '/';  break;
                    case 'u':   //unicode字符
                        // 一口气读取4字节，把这个Unicode字符读取完
                        unsigned int u, u2;
                        if (!parse_hex4(u))  // 返回一个（错误码）不可能的值，直接返回
                            return fail("");
                        /*
                            utf-8的字符长度可以是1，2，3。
                            Json可以使用代理对（surrogate pair），用两个16位的字符来表达一个Unicode码点
                            如果第一个码点是U+D800至U+DBFF，说明这是高代理项，后面一定伴随一个U+DC00至U+DFFF的低代理项
                            这里需要将代理对转换为Unicode码点
                        */
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (str[i++] != '\\')
                                return fail("");
                            if (str[i++] != 'u')
                                return fail("");
                            if (!parse_hex4(u2))
                                return fail("");
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                return fail("");
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(u, out);
                        break;
                    default:
                        return fail("");
                }
                break;
            case '\0':  //字符串没有右引号结尾就意外结束了
                return fail("");
            default:
                if ((unsigned char)ch < 0x20)
                    return fail("");
                out += ch;  //ASCII字符，直接添加
        }
    }
    return fail("");
}

// 解析数组
Json JsonParser::parse_array() {
    Json::array a;
    char ch = get_next_token();
    if (ch == ']') return a;
    while (true) {
        i--;
        a.push_back(parse_json());
        if (a.rbegin()->state != JSON_PARSE_OK)
            return *a.rbegin();
        ch = get_next_token();
        if (ch == ']') break;
        else if (ch != ',') return Json(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET);
        ch = get_next_token();
    }
    return a;
}

// 解析对象
Json JsonParser::parse_object() {
    Json::object o;
    char ch = get_next_token();
    // printf("%c\n", ch);
    if (ch == '}') return o;
    while (true) {
        if (ch != '"') return fail(Json(JSON_PARSE_MISS_KEY));
        string key = parse_string();
        // printf("%s\n", key.c_str());
        if (failed) return fail();
        ch = get_next_token();
        if (ch != ':') return fail(Json(JSON_PARSE_MISS_COLON));
        o[move(key)] = parse_json();
        if (failed) return fail();
        ch = get_next_token();
        if (ch == '}') break;
        if (ch != ',') return fail(Json(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET));
        ch = get_next_token();
    }
    return o;
}

Json Json::parse(const string& in) {
    JsonParser parser(in);
    Json res = parser.parse_json();

    if (parser.get_index() != in.size() || parser.failed)
        return Json(res.state);
    return res;
}

}  // namespace myjson
