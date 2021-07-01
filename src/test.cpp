#include <stdio.h>
#include <iostream>
#include "JSONValue.h"

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
    json_init(&v);
    json_set_bool(&v, 0);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "null"));
    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
    json_free(&v);
}

static void test_parse_true() {
    json_node v;
    json_init(&v);
    json_set_bool(&v, 0);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "true"));
    EXPECT_EQ_INT(JSONTYPE_TRUE, json_get_type(&v));
    json_free(&v);
}

static void test_parse_false() {
    json_node v;
    json_init(&v);
    json_set_bool(&v, 1);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "false"));
    EXPECT_EQ_INT(JSONTYPE_FALSE, json_get_type(&v));
    json_free(&v);
}

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define TEST_NUMBER(expect, json)\
    do {\
        json_node v;\
        json_init(&v);\
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, json_get_number(&v));\
        json_free(&v);\
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
        json_init(&v);
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, json));
        EXPECT_EQ_INT(JSONTYPE_STRING, json_get_type(&v));
        EXPECT_EQ_STRING(expect, json_get_string(&v));
        json_free(&v);
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

    json_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "[ ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, json_get_type(&v));
    EXPECT_EQ_SIZE(0, json_get_array_size(&v));
    json_free(&v);

    json_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, json_get_type(&v));
    EXPECT_EQ_SIZE(5, json_get_array_size(&v));
    EXPECT_EQ_INT(JSONTYPE_NULL,   json_get_type(json_get_array_index(&v, 0)));
    EXPECT_EQ_INT(JSONTYPE_FALSE,  json_get_type(json_get_array_index(&v, 1)));
    EXPECT_EQ_INT(JSONTYPE_TRUE,   json_get_type(json_get_array_index(&v, 2)));
    EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(json_get_array_index(&v, 3)));
    EXPECT_EQ_INT(JSONTYPE_STRING, json_get_type(json_get_array_index(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, json_get_number(json_get_array_index(&v, 3)));
    EXPECT_EQ_STRING("abc", json_get_string(json_get_array_index(&v, 4)));
    json_free(&v);

    json_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, json_get_type(&v));
    EXPECT_EQ_SIZE(4, json_get_array_size(&v));
    for (i = 0; i < 4; i++) {
        json_node* a = json_get_array_index(&v, i);
        EXPECT_EQ_INT(JSONTYPE_ARRAY, json_get_type(a));
        EXPECT_EQ_SIZE(i, json_get_array_size(a));
        for (j = 0; j < i; j++) {
            json_node* e = json_get_array_index(a, j);
            EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, json_get_number(e));
        }
    }
    json_free(&v);
}

static void test_parse_object() {
    json_node v;
    int i;

    json_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, " { } "));
    EXPECT_EQ_INT(JSONTYPE_OBJECT,  json_get_type(&v));
    EXPECT_EQ_SIZE(0, json_get_object_size(&v));
    json_free(&v);

    json_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v,
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
    EXPECT_EQ_INT(JSONTYPE_OBJECT, json_get_type(&v));
    EXPECT_EQ_SIZE(7, json_get_object_size(&v));
    EXPECT_EQ_STRING("n", json_get_object_key(&v, 0));
    EXPECT_EQ_INT(JSONTYPE_NULL,   json_get_type(json_get_object_value(&v, 0)));
    EXPECT_EQ_STRING("f", json_get_object_key(&v, 1));
    EXPECT_EQ_INT(JSONTYPE_FALSE,  json_get_type(json_get_object_value(&v, 1)));
    EXPECT_EQ_STRING("t", json_get_object_key(&v, 2));
    EXPECT_EQ_INT(JSONTYPE_TRUE,   json_get_type(json_get_object_value(&v, 2)));
    EXPECT_EQ_STRING("i", json_get_object_key(&v, 3));
    EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(json_get_object_value(&v, 3)));
    EXPECT_EQ_DOUBLE(123.0, json_get_number(json_get_object_value(&v, 3)));
    EXPECT_EQ_STRING("s", json_get_object_key(&v, 4));
    EXPECT_EQ_INT(JSONTYPE_STRING, json_get_type(json_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("abc", json_get_string(json_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("a", json_get_object_key(&v, 5));
    EXPECT_EQ_INT(JSONTYPE_ARRAY, json_get_type(json_get_object_value(&v, 5)));
    EXPECT_EQ_SIZE(3, json_get_array_size(json_get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        json_node* e = json_get_array_index(json_get_object_value(&v, 5), i);
        EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(e));
        EXPECT_EQ_DOUBLE(i + 1.0, json_get_number(e));
    }
    EXPECT_EQ_STRING("o", json_get_object_key(&v, 6));
    {
        json_node* o = json_get_object_value(&v, 6);
        EXPECT_EQ_INT(JSONTYPE_OBJECT, json_get_type(o));
        for (i = 0; i < 3; i++) {
            json_node* ov = json_get_object_value(o, i);
            EXPECT_TRUE('1' + i == json_get_object_key(o, i)[0]);
            EXPECT_EQ_SIZE(1, json_get_object_key_length(o, i));
            EXPECT_EQ_INT(JSONTYPE_NUMBER, json_get_type(ov));
            EXPECT_EQ_DOUBLE(i + 1.0, json_get_number(ov));
        }
    }
    json_free(&v);
}


#define TEST_PARSE_ERROR(error, json)\
    do {\
        json_node n;\
        json_init(&n);\
        n.type = JSONTYPE_FALSE;\
        EXPECT_EQ_INT(error, json_parse(&n, json));\
        EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&n));\
        json_free(&n);\
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

//static void test_parse_expect_value() {
//    json_node v;
//    v.type = JSONTYPE_FALSE;
//    EXPECT_EQ_INT(JSON_PARSE_EXPECT_VALUE, json_parse(&v, ""));
//    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
//
//    v.type = JSONTYPE_FALSE;
//    EXPECT_EQ_INT(JSON_PARSE_EXPECT_VALUE, json_parse(&v, " "));
//    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
//}
//
//static void test_parse_invalid_value() {
//    json_node v;
//    v.type = JSONTYPE_FALSE;
//    EXPECT_EQ_INT(JSON_PARSE_INVALID_VALUE, json_parse(&v, "nul"));
//    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
//
//    v.type = JSONTYPE_FALSE;
//    EXPECT_EQ_INT(JSON_PARSE_INVALID_VALUE, json_parse(&v, "?"));
//    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
//}

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
        json_init(&v);
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, json));
        json2 = json_stringify(&v);
        EXPECT_EQ_STRING(json, json2);
        json_free(&v);
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

static void TEST_EQUAL(char* json1,char* json2,int equality){
        json_node v1, v2;
        json_init(&v1);
        json_init(&v2);
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v1, json1));
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v2, json2));
        EXPECT_EQ_INT(equality, json_is_equal(&v1, &v2));
        json_free(&v1);
        json_free(&v2);
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
    json_init(&v1);
    json_parse(&v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    json_init(&v2);
    json_copy(&v2, &v1);
    EXPECT_TRUE(json_is_equal(&v2, &v1));
    json_free(&v1);
    json_free(&v2);
}

static void test_move() {
    json_node v1, v2, v3;
    json_init(&v1);
    json_parse(&v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    json_init(&v2);
    json_copy(&v2, &v1);
    json_init(&v3);
    json_move(&v3, &v2);
    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v2));
    EXPECT_TRUE(json_is_equal(&v3, &v1));
    json_free(&v1);
    json_free(&v2);
    json_free(&v3);
}

static void test_swap() {
    json_node v1, v2;
    json_init(&v1);
    json_init(&v2);
    json_set_string(&v1, "Hello");
    json_set_string(&v2, "World!");
    json_swap(&v1, &v2);
    EXPECT_EQ_STRING("World!", json_get_string(&v1));
    EXPECT_EQ_STRING("Hello",  json_get_string(&v2));
    json_free(&v1);
    json_free(&v2);
}

static void test_access_null() {
    json_node v;
    json_init(&v);
    json_set_string(&v, "a");
    json_free(&v);
    EXPECT_EQ_INT(JSONTYPE_NULL, json_get_type(&v));
    json_free(&v);
}
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
static void test_access_boolean() {
    json_node v;
    json_init(&v);
    json_set_string(&v, "a");
    json_set_bool(&v, 1);
    EXPECT_TRUE(json_get_bool(&v));
    json_set_bool(&v, 0);
    EXPECT_FALSE(json_get_bool(&v));
    json_free(&v);
}

static void test_access_number() {
    json_node v;
    json_init(&v);
    json_set_string(&v, "a");
    json_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, json_get_number(&v));
    json_free(&v);
}

static void test_access_string() {
    json_node v;
    json_init(&v);
    json_set_string(&v, "");
    EXPECT_EQ_STRING("", json_get_string(&v));
    json_set_string(&v, "Hello");
    EXPECT_EQ_STRING("Hello", json_get_string(&v));
    json_free(&v);
}

static void test_access_array() {
    json_node* a = new json_node();
    json_node* e = new json_node();
    json_node* n;
    int i, j;
    json_init(a);
    a->type = JSONTYPE_ARRAY;
    for (i = 0; i < 10; i++) {
        n = new json_node();
        json_init(n);
        json_set_number(n, i);
        json_pushback_array_element(a, n);
    }

    EXPECT_EQ_SIZE(10, json_get_array_size(a));
    for (i = 0; i < 10; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    json_popback_array_element(a);
    EXPECT_EQ_SIZE(9, json_get_array_size(a));
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    json_erase_array_element(a, 4, 0);
    EXPECT_EQ_SIZE(9, json_get_array_size(a));
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    json_erase_array_element(a, 8, 1);
    EXPECT_EQ_SIZE(8, json_get_array_size(a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    json_erase_array_element(a, 0, 2);
    EXPECT_EQ_SIZE(6, json_get_array_size(a));
    for (i = 0; i < 6; i++)
        EXPECT_EQ_DOUBLE((double)i + 2, json_get_number(json_get_array_index(a, i)));

    for (i = 0; i < 2; i++) {
        n = new json_node();
        json_init(n);
        json_set_number(n, i);
        json_insert_array_element(a, n, i);
    }

    EXPECT_EQ_SIZE(8, json_get_array_size(a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    EXPECT_TRUE(json_get_array_capacity(a) > 8);
    EXPECT_EQ_SIZE(8, json_get_array_size(a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, json_get_number(json_get_array_index(a, i)));

    n = new json_node();
    json_init(n);
    json_set_string(e, "Hello");
    json_pushback_array_element(a,n);     /* Test if element is freed */

    i = json_get_array_capacity(a);
    json_clear_array(a);
    EXPECT_EQ_SIZE(0, json_get_array_size(a));
    EXPECT_EQ_SIZE(i, json_get_array_capacity(a));   /* capacity remains unchanged */

    json_free(a);
}

static void test_access_object() {
    json_node* o;
    json_node* v;
    json_node* pv;
    int i, j, index;

    json_init(o);
    o->type = JSONTYPE_OBJECT;
    EXPECT_EQ_SIZE(0, json_get_object_size(o));
    EXPECT_EQ_SIZE(j, json_get_object_capacity(o));
    for (i = 0; i < 10; i++) {
        char key[2] = "a";
        key[0] += i;
        v = new json_node();
        json_init(v);
        json_set_number(v, i);
        json_set_object_value(o, key, v);
    }
    EXPECT_EQ_SIZE(10, json_get_object_size(o));
    for (i = 0; i < 10; i++) {
        char key[] = "a";
        key[0] += i;
        index = json_find_object_index(o, key);
        EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
        pv = json_get_object_value(o, index);
        EXPECT_EQ_DOUBLE((double)i, json_get_number(pv));
    }

    index = json_find_object_index(o, "j");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    json_remove_object_value(o, index);
    index = json_find_object_index(o, "j");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ_SIZE(9, json_get_object_size(o));

    index = json_find_object_index(o, "a");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    json_remove_object_value(o, index);
    index = json_find_object_index(o, "a");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ_SIZE(8, json_get_object_size(o));

    EXPECT_TRUE(json_get_object_capacity(o) > 8);

    for (i = 0; i < 8; i++) {
        char key[] = "a";
        key[0] += i + 1;
        EXPECT_EQ_DOUBLE((double)i + 1, json_get_number(json_get_object_value(o, json_find_object_index(o, key))));
    }

    json_set_string(v, "Hello");
    json_set_object_value(o, "World", v); /* Test if element is freed */

    pv = json_find_object_value(o, "World");
    EXPECT_TRUE(pv != NULL);
    EXPECT_EQ_STRING("Hello", json_get_string(pv));

    i = json_get_object_capacity(o);
    json_clear_object(o);
    EXPECT_EQ_SIZE(0, json_get_object_size(o));
    EXPECT_EQ_SIZE(i, json_get_object_capacity(o)); /* capacity remains unchanged */

    json_free(o);
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
