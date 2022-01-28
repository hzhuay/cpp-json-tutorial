#include "myjson.h"
#include <cstdio>

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

class JsonDouble final : public Value<Json::JSON_NUMBER, double> {
    double number_value() const { return value; }
    // 使用static_cast安全转型
    int int_value() const { return static_cast<int>(value); }

   public:
    JsonDouble(double v) : Value(v) {}
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
Json::Json(int value) : v_ptr(make_shared<JsonInt>(value)) {
    puts("int");
}
Json::Json(double value) : v_ptr(make_shared<JsonDouble>(value)) {
    puts("double");
}

// Json的访问器
bool Json::bool_value() const {
    return v_ptr->bool_value();
}

int Json::int_value() const {
    return v_ptr->int_value();
}

double Json::number_value() const {
    return v_ptr->number_value();
}

// JsonParser
Json JsonParser::parse_json() {
    char ch = str[i++];
    switch (ch) {
        case 'n':
            return expect("null", Json());
        case 't':
            return expect("true", Json(true));
        case 'f':
            return expect("false", Json(false));
        default:
            i--;
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
    printf("i = %zu\n", i);

    // 可选的负号部分
    if (str[i] == '-')
        i++;  //跳过负号

    // int部分
    if (str[i] == '0')
        i++;
    else {
        if (!in_range(str[i], '1', '9'))
            return Json(5);  //检查至少有一个digit
        for (i++; in_range(str[i], '0', '9'); i++)
            ;  //跳过所有digit
    }

    if (str[i] != '.' && str[i] != 'e' && str[i] != 'E' &&
        (i - start) <=
            static_cast<size_t>(std::numeric_limits<int>::digits10)) {
        // std::numeric_limits<int>::digits10表示用10进制表示int的最大值需要的位数
        // 将该字符串转化为int
        int res = std::atoi(str.c_str() + start);
        printf("int res = %d\n", res);
        return std::atoi(str.c_str() + start);
    }

    // Decimal部分，从这开始使用double
    if (str[i] == '.') {
        i++;
        if (!in_range(str[i], '0', '9'))  //检查至少有一个digit
            return Json(6);
        for (i++; in_range(str[i], '0', '9'); i++)
            ;
    }

    // Exponent部分
    if (str[i] == 'e' || str[i] == 'E') {
        i++;
        if (str[i] == '+' || str[i] == '-')  //跳过正负号
            i++;
        if (!in_range(str[i], '0', '9'))
            return Json(7);
        for (i++; in_range(str[i], '0', '9'); i++)
            ;
    }
    double res = std::strtod(str.c_str() + start, nullptr);
    printf("double res = %f\n", res);
    return std::strtod(str.c_str() + start, nullptr);
}

Json Json::parse(const std::string& in) {
    JsonParser parser{in};
    Json res = parser.parse_json();
    return res;
}

}  // namespace myjson
