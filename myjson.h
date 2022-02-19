#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

using std::string;
// using std::shared_ptr;

namespace myjson {
enum State {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_ROOT_NOT_SINGULAR,
    JSON_PARSE_NUMBER_TOO_BIG,
    JSON_PARSE_MISS_QUOTATION_MARK,
    JSON_PARSE_INVALID_STRING_ESCAPE,
    JSON_PARSE_INVALID_STRING_CHAR,
    JSON_PARSE_INVALID_UNICODE_HEX,
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    JSON_PARSE_MISS_KEY,
    JSON_PARSE_MISS_COLON,
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

class JsonValue;

class Json {
   private:
    std::shared_ptr<JsonValue> v_ptr;

   public:
    enum Type {
        JSON_NULL,
        JSON_BOOL,
        JSON_NUMBER,
        JSON_STRING,
        JSON_ARRAY,
        JSON_OBJECT
    };

    State state = JSON_PARSE_OK;

    // 给数组和对象类型起别名
    typedef std::vector<Json> array;
    typedef std::map<string, Json> object;

    // 构造函数
    Json() noexcept;
    // Json(std::nullptr_t);
    Json(State s);              // 用State构造
    Json(bool value);           // BOOL
    Json(int value);            // int
    Json(double value);         // double
    Json(const string& value);  // std::string
    Json(string&& value);       // move string
    Json(const char* value);    // c-style string
    Json(const array& value);   // std::vector
    Json(array&& value);        // move array
    Json(const object& value);  // std::map
    Json(object&& value);       // move map


    // 类型
    Type type() const;
    bool is_null()      const { return type() == JSON_NULL; }
    bool is_bool()      const { return type() == JSON_BOOL; }
    bool is_number()    const { return type() == JSON_NUMBER; }
    bool is_string()    const { return type() == JSON_STRING; }
    bool is_array()     const { return type() == JSON_ARRAY; }
    bool is_obejct()    const { return type() == JSON_OBJECT; }

    // 获得值
    bool bool_value() const;
    double number_value() const;
    int int_value() const;
    const string& string_value() const;
    const array & array_value() const;
    const object & object_value() const;

    // 重载[]符号，通过index得到数组元素，或者string得到object元素
    const Json& operator[] (size_t i) const;
    const Json& operator[] (const string& key) const;

    bool operator== (const Json &rhs) const;
    bool operator<  (const Json &rhs) const;
    bool operator!= (const Json &rhs) const { return !(*this == rhs); }
    bool operator<= (const Json &rhs) const { return !(rhs < *this); }
    bool operator>  (const Json &rhs) const { return  (rhs < *this); }
    bool operator>= (const Json &rhs) const { return !(*this < rhs); }

    static Json parse(const std::string& in);
};

class JsonValue {
    friend class Json;

   protected:
    virtual Json::Type type() const = 0;
    virtual bool bool_value() const;
    virtual double number_value() const;
    virtual int int_value() const;
    virtual const string& string_value() const;
    virtual const Json::array& array_value() const;
    virtual const Json::object& object_value() const;
    virtual const Json& operator[] (size_t i) const;
    virtual const Json& operator[] (const string& key) const;

};

class JsonParser final {
private:
    const string& str;
    size_t i = 0;


public:
    bool failed = false;
    JsonParser(const std::string& in) : str(in){};
    Json parse();
    void parse_whitespace();
    char get_next_token();
    Json parse_json();
    Json parse_literal(const string& expected, Json res);
    Json parse_number();
    bool parse_hex4(unsigned int & u);
    string parse_string();
    Json parse_array();
    Json parse_object();
    int get_index() const { return i; }
    void encode_utf8(unsigned int u, string& out);

    template <typename T>
    T fail(const T ret) {
        failed = true;
        return ret;
    }

    Json fail() {
        return fail(Json());
    }
};

}  // namespace myjson
