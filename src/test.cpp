#include <cstdio>
#include <iostream>
#include "JsonValue.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse_null() {
    json_node v;
    v.json_init();
    v.json_set_bool(0);
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse("null"));
    EXPECT_EQ_INT(JSONTYPE_NULL, v.json_get_type());
    v.json_free();
}

static void test_parse_true() {
    json_node v;
    v.json_init();
    v.json_set_bool(0);
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse("true"));
    EXPECT_EQ_INT(JSONTYPE_TRUE, v.json_get_type());
    v.json_free();
}

static void test_parse_false() {
    json_node v;
    v.json_init();
    v.json_set_bool(1);
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse("false"));
    EXPECT_EQ_INT(JSONTYPE_FALSE, v.json_get_type());
    v.json_free();
}

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define TEST_NUMBER(expect, json)\
    do {\
        json_node v;\
        v.json_init();\
        EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse(json));\
        EXPECT_EQ_INT(JSONTYPE_NUMBER, v.json_get_type());\
        EXPECT_EQ_DOUBLE(expect, v.json_get_number());\
        v.json_free();\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void EXPECT_EQ_STRING(std::string expect, std::string actual){
    test_count++;
    if (expect == actual)
        test_pass++;
    else {
        fprintf(stderr, "%s:%d:\n", __FILE__, __LINE__);
        std::cout <<  "expect: " << expect << " actual: " << actual << std::endl;
        main_ret = 1;
    }
}

static void TEST_STRING(std::string expect, char* json){
    json_node v;
    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse(json));
    EXPECT_EQ_INT(JSONTYPE_STRING, v.json_get_type());
    EXPECT_EQ_STRING(expect, v.json_get_string());
    v.json_free();
}

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    /* 此处为了实现输入的string不会被\0截断，expect为"Hello\0World" */
    std::string str_test = "Hello";
    str_test += '\0';
    str_test += "World";
    TEST_STRING(str_test, "\"Hello\\u0000World\""); /* 比较时，被\0截断 */
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

#define EXPECT_EQ_SIZE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")

static void test_parse_array() {
    int i, j;
    json_node v;

    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse( "[ ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, v.json_get_type());
    EXPECT_EQ_SIZE(0, v.json_get_array_size());
    v.json_free();

    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse("[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, v.json_get_type());
    EXPECT_EQ_SIZE(5, v.json_get_array_size());
    EXPECT_EQ_INT(JSONTYPE_NULL,   v.json_get_array_index(0)->json_get_type());
    EXPECT_EQ_INT(JSONTYPE_FALSE,  v.json_get_array_index(1)->json_get_type());
    EXPECT_EQ_INT(JSONTYPE_TRUE,   v.json_get_array_index(2)->json_get_type());
    EXPECT_EQ_INT(JSONTYPE_NUMBER, v.json_get_array_index(3)->json_get_type());
    EXPECT_EQ_INT(JSONTYPE_STRING, v.json_get_array_index(4)->json_get_type());
    EXPECT_EQ_DOUBLE(123.0, v.json_get_array_index(3)->json_get_number());
    EXPECT_EQ_STRING("abc", v.json_get_array_index(4)->json_get_string());
    v.json_free();

    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, v.json_get_type());
    EXPECT_EQ_SIZE(4, v.json_get_array_size());
    for (i = 0; i < 4; i++) {
        json_node* a = v.json_get_array_index(i);
        EXPECT_EQ_INT(JSONTYPE_ARRAY, a->json_get_type());
        EXPECT_EQ_SIZE(i, a->json_get_array_size());
        for (j = 0; j < i; j++) {
            json_node* e = a->json_get_array_index(j);
            EXPECT_EQ_INT(JSONTYPE_NUMBER, e->json_get_type());
            EXPECT_EQ_DOUBLE((double)j, e->json_get_number());
        }
    }
    v.json_free();
}

static void test_parse_object() {
    json_node v;
    int i;

    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse(" { } "));
    EXPECT_EQ_INT(JSONTYPE_OBJECT,  v.json_get_type());
    EXPECT_EQ_SIZE(0, v.json_get_object_size());
    v.json_free();

    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse(
                                            " { "
                                            "\"n\" : null , "
                                            "\"f\" : false , "
                                            "\"t\" : true , "
                                            "\"i\" : 123 , "
                                            "\"s\" : \"abc\", "
                                            "\"a\" : [ 1, 2, 3 ],"
                                            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                            " } "
    ));
    EXPECT_EQ_INT(JSONTYPE_OBJECT, v.json_get_type());
    EXPECT_EQ_SIZE(7, v.json_get_object_size());
    EXPECT_EQ_STRING("n", v.json_get_object_key(0));
    EXPECT_EQ_INT(JSONTYPE_NULL,   v.json_get_object_value(0)->json_get_type());
    EXPECT_EQ_STRING("f", v.json_get_object_key(1));
    EXPECT_EQ_INT(JSONTYPE_FALSE,  v.json_get_object_value(1)->json_get_type());
    EXPECT_EQ_STRING("t", v.json_get_object_key(2));
    EXPECT_EQ_INT(JSONTYPE_TRUE,   v.json_get_object_value(2)->json_get_type());
    EXPECT_EQ_STRING("i", v.json_get_object_key(3));
    EXPECT_EQ_INT(JSONTYPE_NUMBER, v.json_get_object_value(3)->json_get_type());
    EXPECT_EQ_DOUBLE(123.0, v.json_get_object_value(3)->json_get_number());
    EXPECT_EQ_STRING("s", v.json_get_object_key(4));
    EXPECT_EQ_INT(JSONTYPE_STRING, v.json_get_object_value(4)->json_get_type());
    EXPECT_EQ_STRING("abc", v.json_get_object_value(4)->json_get_string());
    EXPECT_EQ_STRING("a", v.json_get_object_key(5));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, v.json_get_object_value(5)->json_get_type());
    EXPECT_EQ_SIZE(3, v.json_get_object_value(5)->json_get_array_size());
    for (i = 0; i < 3; i++) {
        json_node* e = v.json_get_object_value(5)->json_get_array_index(i);
        EXPECT_EQ_INT(JSONTYPE_NUMBER, e->json_get_type());
        EXPECT_EQ_DOUBLE(i + 1.0, e->json_get_number());
    }
    EXPECT_EQ_STRING("o", v.json_get_object_key(6));
    {
        json_node* o = v.json_get_object_value(6);
        EXPECT_EQ_INT(JSONTYPE_OBJECT, o->json_get_type());
        for (i = 0; i < 3; i++) {
            json_node* ov = o->json_get_object_value(i);
            EXPECT_TRUE('1' + i == o->json_get_object_key(i)[0]);
            EXPECT_EQ_SIZE(1, o->json_get_object_key_length(i));
            EXPECT_EQ_INT(JSONTYPE_NUMBER, ov->json_get_type());
            EXPECT_EQ_DOUBLE(i + 1.0, ov->json_get_number());
        }
    }
    v.json_free();
}


#define TEST_PARSE_ERROR(error, json)\
    do {\
        json_node n;\
        n.json_init();\
        n.json_set_bool(0);\
        EXPECT_EQ_INT(error, n.json_parse(json));\
        EXPECT_EQ_INT(JSONTYPE_NULL, n.json_get_type());\
        n.json_free();\
    } while(0)

static void test_parse_expect_value() {
    TEST_PARSE_ERROR(JSON_PARSE_EXPECT_VALUE, "");
    TEST_PARSE_ERROR(JSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "nul");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "nan");

    /* invalid value in array */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "[1,]");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_VALUE, "[\"a\", nul]");
}

static void test_parse_root_not_singular() {
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "null x");

    /* invalid number */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0123"); /* after zero should be '.' or nothing */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0x0");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0x123");
}

static void test_parse_number_too_big() {
    TEST_PARSE_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_PARSE_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

//缺失引号
static void test_parse_miss_quotation_mark() {
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DoubleDuote, "\"");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DoubleDuote, "\"abc");
}

//非法转义符
static void test_parse_invalid_string_escape() {
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\v\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\'\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\0\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}
//非法unicode
static void test_parse_invalid_unicode_hex() {
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}
//缺失中括号
static void test_parse_miss_comma_or_square_bracket() {
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

//缺失key值
static void test_parse_miss_key() {
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{1:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{true:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{false:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{null:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{[]:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{{}:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_miss_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
}

static void TEST_ROUNDTRIP(char* json){
    json_node v;
    std::string json2;
    int length;
    v.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v.json_parse(json));
    json2 = v.json_stringify();
    EXPECT_EQ_STRING(json, json2);
    v.json_free();
}

static void test_stringify_number() {
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
    TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    TEST_ROUNDTRIP("-4.9406564584124654e-324");
    TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
    TEST_ROUNDTRIP("-2.2250738585072009e-308");
    TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
    TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

static void TEST_EQUAL(char* json1, char* json2, int equality){
    json_node v1, v2;
    v1.json_init();
    v2.json_init();
    EXPECT_EQ_INT(JSON_PARSE_OK, v1.json_parse(json1));
    EXPECT_EQ_INT(JSON_PARSE_OK, v2.json_parse(json2));
    EXPECT_EQ_INT(equality, v1.json_is_equal(&v2));
    v1.json_free();
    v2.json_free();
}


static void test_equal() {
    TEST_EQUAL("true", "true", 1);
    TEST_EQUAL("true", "false", 0);
    TEST_EQUAL("false", "false", 1);
    TEST_EQUAL("null", "null", 1);
    TEST_EQUAL("null", "0", 0);
    TEST_EQUAL("123", "123", 1);
    TEST_EQUAL("123", "456", 0);
    TEST_EQUAL("\"abc\"", "\"abc\"", 1);
    TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
    TEST_EQUAL("[]", "[]", 1);
    TEST_EQUAL("[]", "null", 0);
    TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
    TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
    TEST_EQUAL("[[]]", "[[]]", 1);
    TEST_EQUAL("{}", "{}", 1);
    TEST_EQUAL("{}", "null", 0);
    TEST_EQUAL("{}", "[]", 0);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

static void test_copy() {
    json_node v1, v2;
    v1.json_init();
    v1.json_parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    v2.json_init();
    v2.json_copy(&v1);
    EXPECT_TRUE(v2.json_is_equal(&v1));
    v1.json_free();
    v2.json_free();
}

static void test_move() {
    json_node v1, v2, v3;
    v1.json_init();
    v1.json_parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    v2.json_init();
    v2.json_copy(&v1);
    v3.json_init();
    v3.json_move(&v2);
    EXPECT_EQ_INT(JSONTYPE_NULL, v2.json_get_type());
    EXPECT_TRUE(v3.json_is_equal(&v1));
    v1.json_free();
    v2.json_free();
    v3.json_free();
}

static void test_swap() {
    json_node v1, v2;
    v1.json_init();
    v2.json_init();
    v1.json_set_string("Hello");
    v2.json_set_string("World!");
    v1.json_swap(&v2);
    EXPECT_EQ_STRING("World!", v1.json_get_string());
    EXPECT_EQ_STRING("Hello",  v2.json_get_string());
    v1.json_free();
    v2.json_free();
}

static void test_access_null() {
    json_node v;
    v.json_init();
    v.json_set_string("a");
    v.json_free();
    EXPECT_EQ_INT(JSONTYPE_NULL, v.json_get_type());
    v.json_free();
}
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
static void test_access_boolean() {
    json_node v;
    v.json_init();
    v.json_set_string("a");
    v.json_set_bool(1);
    EXPECT_TRUE(v.json_get_bool());
    v.json_set_bool(0);
    EXPECT_FALSE(v.json_get_bool());
    v.json_free();
}

static void test_access_number() {
    json_node v;
    v.json_init();
    v.json_set_string("a");
    v.json_set_number(1234.5);
    EXPECT_EQ_DOUBLE(1234.5, v.json_get_number());
    v.json_free();
}

static void test_access_string() {
    json_node v;
    v.json_init();
    v.json_set_string("");
    EXPECT_EQ_STRING("", v.json_get_string());
    v.json_set_string("Hello");
    EXPECT_EQ_STRING("Hello", v.json_get_string());
    v.json_free();
}

static void test_access_array() {
    json_node* a = new json_node();
    json_node* e = new json_node();
    json_node* n;
    int i, j;
    a->json_init();
    a->json_set_array();
    for (i = 0; i < 10; i++) {
        n = new json_node();
        n->json_init();
        n->json_set_number(i);
        a->json_pushback_array_element(n);
    }

    EXPECT_EQ_SIZE(10, a->json_get_array_size());
    for (i = 0; i < 10; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    a->json_popback_array_element();
    EXPECT_EQ_SIZE(9, a->json_get_array_size());
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(4, 0);
    EXPECT_EQ_SIZE(9, a->json_get_array_size());
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(8, 1);
    EXPECT_EQ_SIZE(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(0, 2);
    EXPECT_EQ_SIZE(6, a->json_get_array_size());
    for (i = 0; i < 6; i++)
        EXPECT_EQ_DOUBLE((double)i + 2, a->json_get_array_index(i)->json_get_number());

    for (i = 0; i < 2; i++) {
        n = new json_node();
        n->json_init();
        n->json_set_number(i);
        a->json_insert_array_element(n, i);
    }

    EXPECT_EQ_SIZE(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    EXPECT_TRUE(a->json_get_array_capacity() > 8);
    EXPECT_EQ_SIZE(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, a->json_get_array_index(i)->json_get_number());

    n = new json_node();
    n->json_init();
    e->json_set_string("Hello");
    a->json_pushback_array_element(n);     /* Test if element is freed */

    i = a->json_get_array_capacity();
    a->json_clear_array();
    EXPECT_EQ_SIZE(0, a->json_get_array_size());
    EXPECT_EQ_SIZE(i, a->json_get_array_capacity());   /* capacity remains unchanged */

    a->json_free();
}

static void test_access_object() {
    json_node* o;
    json_node* v;
    json_node* pv;
    int i, j, index;

    o->json_init();
    o->json_set_object();
    EXPECT_EQ_SIZE(0, o->json_get_object_size());
    EXPECT_EQ_SIZE(j, o->json_get_object_capacity());
    for (i = 0; i < 10; i++) {
        char key[2] = "a";
        key[0] += i;
        v = new json_node();
        v->json_init();
        v->json_set_number(i);
        o->json_set_object_value(key, v);
    }
    EXPECT_EQ_SIZE(10, o->json_get_object_size());
    for (i = 0; i < 10; i++) {
        char key[] = "a";
        key[0] += i;
        index = o->json_find_object_index(key);
        EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
        pv = o->json_get_object_value(index);
        EXPECT_EQ_DOUBLE((double)i, pv->json_get_number());
    }

    index = o->json_find_object_index("j");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->json_remove_object_value(index);
    index = o->json_find_object_index("j");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ_SIZE(9, o->json_get_object_size());

    index = o->json_find_object_index("a");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->json_remove_object_value(index);
    index = o->json_find_object_index("a");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ_SIZE(8, o->json_get_object_size());

    EXPECT_TRUE(o->json_get_object_capacity() > 8);

    for (i = 0; i < 8; i++) {
        char key[] = "a";
        key[0] += i + 1;
        EXPECT_EQ_DOUBLE((double)i + 1, o->json_get_object_value(o->json_find_object_index(key))->json_get_number());
    }

    v->json_set_string("Hello");
    o->json_set_object_value("World", v); /* Test if element is freed */

    pv = o->json_find_object_value("World");
    EXPECT_TRUE(pv != NULL);
    EXPECT_EQ_STRING("Hello", pv->json_get_string());

    i = o->json_get_object_capacity();
    o->json_clear_object();
    EXPECT_EQ_SIZE(0, o->json_get_object_size());
    EXPECT_EQ_SIZE(i, o->json_get_object_capacity()); /* capacity remains unchanged */

    o->json_free();
}

static void test_access() {
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
    test_access_array();
    test_access_object();
}

int main() {
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_parse();
    test_stringify();
    test_equal();
    test_copy();
    test_move();
    test_swap();
    test_access();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}

