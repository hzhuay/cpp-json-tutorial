#pragma once
#include <string>

using std::string;
using std::shared_ptr;

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

    // 构造函数
    Json();
    Json(State s);     // 用State构造
    Json(bool value);  // BOOL
    Json(int value);
    Json(double value);
    Json(const string& value);  // std::string
    Json(string&& value);       // move
    Json(const char* value);     // c-style string

    // 类型
    Type type() const;
    bool is_bool()      const { return type() == JSON_BOOL; }
    bool is_number()    const { return type() == JSON_NUMBER; }
    bool is_string()    const { return type() == JSON_STRING;}

    // 获得值
    bool bool_value() const;
    double number_value() const;
    int int_value() const;
    const string& string_value() const;

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
};

class JsonParser final {
   private:
    const string& str;
    size_t i = 0;

   public:
    JsonParser(const std::string& in) : str(in){};
    Json parse();
    void parse_whitespace();
    char get_next_token();
    Json parse_json();
    Json parse_literal(const string& expected, Json res);
    Json parse_number();
    bool parse_hex4(unsigned int & u);
    Json parse_string();
    Json parse_array();
    Json parse_object();
    int get_index() const { return i; }
    void encode_utf8(unsigned int u, string& out);
};

}  // namespace myjson
