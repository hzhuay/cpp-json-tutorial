#include "myjson.h"

// JsonValue的各种派生类和模板特化
namespace myjson {

using std::make_shared;
using std::move;
using std::shared_ptr;

template <Json::Type t, typename T>
class Value : public JsonValue {
   protected:
    const T value;

    Value(const T& v) : value(v) {}
    Value(T&& v) : value(move(v)) {}

    Json::Type type() const override { return t; }
};

class JsonNull final : public Value<Json::JSON_NULL, nullptr_t> {
   public:
    JsonNull() : Value({}) {}
};

class JsonBool final : public Value<Json::JSON_BOOL, bool> {
    bool bool_value() const { return value; }

   public:
    JsonBool(bool v) : Value(v) {}
};

class JsonInt final : public Value<Json::JSON_NUMBER, int> {
    double number_value() const { return value; }
    int int_value() const { return value; }

   public:
    JsonInt(int v) : Value(v) {}
};

class JsonDouble final : public Value<Json::JSON_NUMBER, int> {
    double number_value() const { return value; }
    // 使用static_cast安全转型
    int int_value() const { return static_cast<int>(value); }

   public:
    JsonDouble(int v) : Value(v) {}
};

// 使用单例模式，创建一系列可以公用的静态值和动态值的初始值。
struct Singleton {
    const shared_ptr<JsonValue> null = make_shared<JsonNull>();
    const shared_ptr<JsonValue> t = make_shared<JsonBool>(true);
    const shared_ptr<JsonValue> f = make_shared<JsonBool>(false);
    Singleton() {}
};

// 利用静态局部变量，实现单例
static const Singleton& singleton() {
    static const Singleton s{};
    return s;
}

}  // namespace myjson

namespace myjson {

Json::Type Json::type() const {
    return v_ptr->type();
}

// Json的一系列构造函数
Json::Json() : v_ptr(singleton().null) {}
Json::Json(bool value) : v_ptr(make_shared<JsonBool>(value)) {}

// Json的访问器
bool Json::bool_value() const {
    return v_ptr->bool_value();
}

// JsonValue的默认访问器
bool JsonValue::bool_value() const {
    return false;
}

// JsonParser
Json JsonParser::parse_json() {
    char ch = str[i++];
    switch (ch) {
        case 'n':
            return expect("null", Json());
            break;
        case 't':
            return expect("true", Json(true));
            break;
        case 'f':
            return expect("false", Json(false));
            break;
        case '\0':
            return Json();
            break;
        default:
            return parse_number();
    }
}

// 解析null, boll
Json JsonParser::expect(const string& expected, Json res) {
    assert(i != 0);
    i--;
    if (str.compare(i, expected.length(), expected) == 0) {
        i += expected.length();
        return res;
    } else {
        return Json();
    }
}

static inline bool in_range(long x, long lower, long upper) {
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
    if (str[i] == '-')
        i++;  //跳过负号

    // int部分
    if (str[i] == '0')
        i++;
    else {
        if (!in_range(str[i], '1', '9'))
            return Json();  //检查至少有一个digit
        for (i++; in_range(str[i], '0', '9'); i++)
            ;  //跳过所有digit
    }
    if (str[i] == '.') {
        i++;
        if (!in_range(str[i], '0', '9'))
            return Json();  //检查至少有一个digit
    }
}

Json Json::parse(const std::string& in) {
    JsonParser parser{in};
    Json res = parser.parse_json();
    return res;
}

}  // namespace myjson
