#pragma once
#include <string>

using std::string;

namespace myjson {
enum {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_ROOT_NOT_SINGULAR
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

    // 构造函数
    Json();
    Json(bool value);  // BOOL

    // 类型
    Type type() const;
    bool is_bool() const { return type() == JSON_BOOL; }

    // 获得值
    bool bool_value() const;

    static Json parse(const std::string& in);
};

class JsonValue {
    friend class Json;

   protected:
    virtual Json::Type type() const = 0;
    virtual bool bool_value() const;
};

class JsonParser final {
   private:
    const string& str;
    size_t i = 0;

   public:
    JsonParser(const std::string& in) : str(in) {}
    Json parse_json();
    Json expect(const string& expected, Json res);
    Json parse_number();
};

}  // namespace myjson
