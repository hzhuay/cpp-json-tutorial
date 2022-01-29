#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "myjson.h"

// static int main_ret = 0;
// static int test_count = 0;
// static int test_pass = 0;

using namespace myjson;
using std::cin;
using std::cout;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                      \
    do {                                                                      \
        test_count++;                                                         \
        if (equality)                                                         \
            test_pass++;                                                      \
        else {                                                                \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
                    __FILE__, __LINE__, expect, actual);                      \
            main_ret = 1;                                                     \
        }                                                                     \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse() {
    Json json = Json::parse("true");
    cout << json.bool_value() << std::endl;

    json = Json::parse("false");
    cout << json.bool_value() << std::endl;

    json = Json::parse("null");
    cout << json.bool_value() << std::endl;

    json = Json::parse("-10");
    cout << json.number_value() << std::endl;

    json = Json::parse("1.12");
    cout << json.number_value() << std::endl;

    json = Json::parse("1.12e-10");
    cout << json.number_value() << std::endl;

    json = Json::parse("\"Hello World 你好，世界\"");
    cout << json.string_value() << std::endl;

    // std::cout << "k1: " << json["k1"].string_value() << "\n";
    // std::cout << "k3: " << json["k3"].dump() << "\n";
}

int main() {
    test_parse();
    // printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count,test_pass *
    // 100.0 / test_count);
    return 0;
}
