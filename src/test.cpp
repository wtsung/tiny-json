#include "JsonNode.h"
#include <gtest/gtest.h>

TEST(Test_Json, test_parse_null) {
    json_node v;
    v.json_init();
    v.json_set_bool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.json_parse("null"));
    EXPECT_EQ(JSONTYPE_NULL, v.json_get_type());
    v.json_free();
}

TEST(Test_Json, test_parse_true) {
    json_node v;
    v.json_init();
    v.json_set_bool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.json_parse("true"));
    EXPECT_EQ(JSONTYPE_TRUE, v.json_get_type());
    v.json_free();
}

TEST(Test_Json, test_parse_false) {
    json_node v;
    v.json_init();
    v.json_set_bool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.json_parse("false"));
    EXPECT_EQ(JSONTYPE_FALSE, v.json_get_type());
    v.json_free();
}

#define TEST_PARSE_NUMBER(expect_num, json)                \
    do {                                                   \
        json_node n;                                       \
        n.json_init();                                     \
        n.json_set_bool(0);                                \
        EXPECT_EQ(JSON_PARSE_OK, n.json_parse(json));      \
        EXPECT_EQ(JSONTYPE_NUMBER, n.json_get_type());     \
        EXPECT_DOUBLE_EQ(expect_num, n.json_get_number()); \
        n.json_free();                                     \
    } while (0)

TEST(Test_Json, test_parse_number) {
    TEST_PARSE_NUMBER(0.0, "0");
    TEST_PARSE_NUMBER(0.0, "-0");
    TEST_PARSE_NUMBER(0.0, "-0.0");
    TEST_PARSE_NUMBER(1.0, "1");
    TEST_PARSE_NUMBER(-1.0, "-1");
    TEST_PARSE_NUMBER(1.5, "1.5");
    TEST_PARSE_NUMBER(-1.5, "-1.5");
    TEST_PARSE_NUMBER(3.1416, "3.1416");
    TEST_PARSE_NUMBER(1E10, "1E10");
    TEST_PARSE_NUMBER(1e10, "1e10");
    TEST_PARSE_NUMBER(1E+10, "1E+10");
    TEST_PARSE_NUMBER(1E-10, "1E-10");
    TEST_PARSE_NUMBER(-1E10, "-1E10");
    TEST_PARSE_NUMBER(-1e10, "-1e10");
    TEST_PARSE_NUMBER(-1E+10, "-1E+10");
    TEST_PARSE_NUMBER(-1E-10, "-1E-10");
    TEST_PARSE_NUMBER(1.234E+10, "1.234E+10");
    TEST_PARSE_NUMBER(1.234E-10, "1.234E-10");
    TEST_PARSE_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_PARSE_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_PARSE_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_PARSE_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_PARSE_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_PARSE_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_PARSE_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_PARSE_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_PARSE_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); /* Max double */
    TEST_PARSE_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_PARSE_STRING(expect_string, json)         \
    do {                                               \
        json_node n;                                   \
        n.json_init();                                 \
        EXPECT_EQ(JSON_PARSE_OK, n.json_parse(json));  \
        EXPECT_EQ(JSONTYPE_STRING, n.json_get_type()); \
        EXPECT_EQ(expect_string, n.json_get_string()); \
        n.json_free();                                 \
    } while (0)

TEST(Test_Json, test_parse_string) {
    TEST_PARSE_STRING("", "\"\"");
    TEST_PARSE_STRING("Hello", "\"Hello\"");
    TEST_PARSE_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_PARSE_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    std::string str = "Hello";
    str += '\0';
    str += "World";
    TEST_PARSE_STRING(str, "\"Hello\\u0000World\"");
    TEST_PARSE_STRING("\x24", "\"\\u0024\"");                    /* Dollar sign U+0024 */
    TEST_PARSE_STRING("\xC2\xA2", "\"\\u00A2\"");                /* Cents sign U+00A2 */
    TEST_PARSE_STRING("\xE2\x82\xAC", "\"\\u20AC\"");            /* Euro sign U+20AC */
    TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
    TEST_PARSE_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
}

TEST(Test_Json, test_parse_array) {
    json_node n;
    n.json_init();
    EXPECT_EQ(JSON_PARSE_OK, n.json_parse("[ ]"));
    EXPECT_EQ(JSONTYPE_ARRAY, n.json_get_type());
    EXPECT_EQ(0, n.json_get_array_size());
    n.json_free();

    EXPECT_EQ(JSON_PARSE_OK, n.json_parse("[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(JSONTYPE_ARRAY, n.json_get_type());
    EXPECT_EQ(5, n.json_get_array_size());
    EXPECT_EQ(JSONTYPE_NULL, n.json_get_array_index(0)->json_get_type());
    EXPECT_EQ(JSONTYPE_FALSE, n.json_get_array_index(1)->json_get_type());
    EXPECT_EQ(JSONTYPE_TRUE, n.json_get_array_index(2)->json_get_type());
    EXPECT_EQ(JSONTYPE_NUMBER, n.json_get_array_index(3)->json_get_type());
    EXPECT_EQ(JSONTYPE_STRING, n.json_get_array_index(4)->json_get_type());
    EXPECT_DOUBLE_EQ(123.0, n.json_get_array_index(3)->json_get_number());
    EXPECT_EQ("abc", n.json_get_array_index(4)->json_get_string());
    n.json_free();

    n.json_init();
    EXPECT_EQ(JSON_PARSE_OK, n.json_parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(JSONTYPE_ARRAY, n.json_get_type());
    EXPECT_EQ(4, n.json_get_array_size());
    for (int i = 0; i < 4; i++) {
        json_node* a = n.json_get_array_index(i);
        EXPECT_EQ(JSONTYPE_ARRAY, a->json_get_type());
        EXPECT_EQ(i, a->json_get_array_size());
        for (int j = 0; j < i; j++) {
            json_node* e = a->json_get_array_index(j);
            EXPECT_EQ(JSONTYPE_NUMBER, e->json_get_type());
            EXPECT_DOUBLE_EQ((double) j, e->json_get_number());
        }
    }
    n.json_free();
}

TEST(Test_Json, test_parse_object) {
    json_node n;
    int i;

    n.json_init();
    EXPECT_EQ(JSON_PARSE_OK, n.json_parse(" { } "));
    EXPECT_EQ(JSONTYPE_OBJECT, n.json_get_type());
    EXPECT_EQ(0, n.json_get_object_size());
    n.json_free();

    n.json_init();
    EXPECT_EQ(JSON_PARSE_OK, n.json_parse(
                                     " { "
                                     "\"n\" : null , "
                                     "\"f\" : false , "
                                     "\"t\" : true , "
                                     "\"i\" : 123 , "
                                     "\"s\" : \"abc\", "
                                     "\"a\" : [ 1, 2, 3 ],"
                                     "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                     " } "));
    EXPECT_EQ(JSONTYPE_OBJECT, n.json_get_type());
    EXPECT_EQ(7, n.json_get_object_size());
    EXPECT_EQ("n", n.json_get_object_key(0));
    EXPECT_EQ(JSONTYPE_NULL, n.json_get_object_value(0)->json_get_type());
    EXPECT_EQ("f", n.json_get_object_key(1));
    EXPECT_EQ(JSONTYPE_FALSE, n.json_get_object_value(1)->json_get_type());
    EXPECT_EQ("t", n.json_get_object_key(2));
    EXPECT_EQ(JSONTYPE_TRUE, n.json_get_object_value(2)->json_get_type());
    EXPECT_EQ("i", n.json_get_object_key(3));
    EXPECT_EQ(JSONTYPE_NUMBER, n.json_get_object_value(3)->json_get_type());
    EXPECT_DOUBLE_EQ(123.0, n.json_get_object_value(3)->json_get_number());
    EXPECT_EQ("s", n.json_get_object_key(4));
    EXPECT_EQ(JSONTYPE_STRING, n.json_get_object_value(4)->json_get_type());
    EXPECT_EQ("abc", n.json_get_object_value(4)->json_get_string());
    EXPECT_EQ("a", n.json_get_object_key(5));
    EXPECT_EQ(JSONTYPE_ARRAY, n.json_get_object_value(5)->json_get_type());
    EXPECT_EQ(3, n.json_get_object_value(5)->json_get_array_size());
    for (i = 0; i < 3; i++) {
        json_node* e = n.json_get_object_value(5)->json_get_array_index(i);
        EXPECT_EQ(JSONTYPE_NUMBER, e->json_get_type());
        EXPECT_DOUBLE_EQ(i + 1.0, e->json_get_number());
    }
    EXPECT_EQ("o", n.json_get_object_key(6));
    {
        json_node* o = n.json_get_object_value(6);
        EXPECT_EQ(JSONTYPE_OBJECT, o->json_get_type());
        for (i = 0; i < 3; i++) {
            json_node* ov = o->json_get_object_value(i);
            EXPECT_TRUE('1' + i == o->json_get_object_key(i)[0]);
            EXPECT_EQ(1, o->json_get_object_key_length(i));
            EXPECT_EQ(JSONTYPE_NUMBER, ov->json_get_type());
            EXPECT_DOUBLE_EQ(i + 1.0, ov->json_get_number());
        }
    }
    n.json_free();
}


#define TEST_PARSE_ERROR(error, json)                \
    do {                                             \
        json_node n;                                 \
        n.json_init();                               \
        n.json_set_bool(0);                          \
        EXPECT_EQ(error, n.json_parse(json));        \
        EXPECT_EQ(JSONTYPE_NULL, n.json_get_type()); \
        n.json_free();                               \
    } while (0)

TEST(Test_Json, test_parse_error) {
    /* expect value */
    TEST_PARSE_ERROR(JSON_PARSE_EXPECT_VALUE, "");
    TEST_PARSE_ERROR(JSON_PARSE_EXPECT_VALUE, " ");

    /* invalid value */
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

    /* root not singular */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "null x");
    /* invalid number */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0123"); /* after zero should be '.' or nothing */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0x0");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_SINGLE_VALUE, "0x123");

    /* number too big */
    TEST_PARSE_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_PARSE_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "-1e309");

    /* missing quotes */
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DoubleDuote, "\"");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DoubleDuote, "\"abc");

    /* invalid escape character */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\v\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\'\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\0\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_ESCAPEVALUE, "\"\\x12\"");

    /* invalid string char */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");

    /* invalid unicode */
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

    /* invalid unicode surrogate */
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_PARSE_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");

    /* missing square brackets */
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");

    /* missing key value */
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{1:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{true:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{false:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{null:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{[]:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{{}:1,");
    TEST_PARSE_ERROR(JSON_PARSE_NOT_EXIST_KEY, "{\"a\":1,");

    /* miss colon */
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");

    /* miss comma or curly bracket */
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}


#define TEST_STRINGIFY(json)                          \
    do {                                              \
        json_node n;                                  \
        std::string json2;                            \
        int length;                                   \
        n.json_init();                                \
        EXPECT_EQ(JSON_PARSE_OK, n.json_parse(json)); \
        json2 = n.json_stringify();                   \
        EXPECT_EQ(json, json2);                       \
        n.json_free();                                \
    } while (0)

TEST(Test_Json, test_stringify) {
    TEST_STRINGIFY("null");
    TEST_STRINGIFY("false");
    TEST_STRINGIFY("true");

    /* stringify number */
    TEST_STRINGIFY("0");
    TEST_STRINGIFY("-0");
    TEST_STRINGIFY("1");
    TEST_STRINGIFY("-1");
    TEST_STRINGIFY("1.5");
    TEST_STRINGIFY("-1.5");
    TEST_STRINGIFY("3.25");
    TEST_STRINGIFY("1e+20");
    TEST_STRINGIFY("1.234e+20");
    TEST_STRINGIFY("1.234e-20");

    TEST_STRINGIFY("1.0000000000000002");      /* the smallest number > 1 */
    TEST_STRINGIFY("4.9406564584124654e-324"); /* minimum denormal */
    TEST_STRINGIFY("-4.9406564584124654e-324");
    TEST_STRINGIFY("2.2250738585072009e-308"); /* Max subnormal double */
    TEST_STRINGIFY("-2.2250738585072009e-308");
    TEST_STRINGIFY("2.2250738585072014e-308"); /* Min normal positive double */
    TEST_STRINGIFY("-2.2250738585072014e-308");
    TEST_STRINGIFY("1.7976931348623157e+308"); /* Max double */
    TEST_STRINGIFY("-1.7976931348623157e+308");

    /* stringify string */
    TEST_STRINGIFY("\"\"");
    TEST_STRINGIFY("\"Hello\"");
    TEST_STRINGIFY("\"Hello\\nWorld\"");
    TEST_STRINGIFY("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_STRINGIFY("\"Hello\\u0000World\"");

    /* stringify array */
    TEST_STRINGIFY("[]");
    TEST_STRINGIFY("[null,false,true,123,\"abc\",[1,2,3]]");

    /* stringify object */
    TEST_STRINGIFY("{}");
    TEST_STRINGIFY("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

#define TEST_EQUAL(json1, json2, equality)              \
    do {                                                \
        json_node n1, n2;                               \
        n1.json_init();                                 \
        n2.json_init();                                 \
        EXPECT_EQ(JSON_PARSE_OK, n1.json_parse(json1)); \
        EXPECT_EQ(JSON_PARSE_OK, n2.json_parse(json2)); \
        EXPECT_EQ(equality, n1.json_is_equal(&n2));     \
        n1.json_free();                                 \
        n2.json_free();                                 \
    } while (0)

TEST(Test_Json, test_equal) {
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

TEST(Test_Json, test_copy) {
    json_node n1, n2;
    n1.json_init();
    n1.json_parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    n2.json_init();
    n2.json_copy(&n1);
    EXPECT_TRUE(n2.json_is_equal(&n1));
    n1.json_free();
    n2.json_free();
}

TEST(Test_Json, test_move) {
    json_node n1, n2, n3;
    n1.json_init();
    n1.json_parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    n2.json_init();
    n2.json_copy(&n1);
    n3.json_init();
    n3.json_move(&n2);
    EXPECT_EQ(JSONTYPE_NULL, n2.json_get_type());
    EXPECT_TRUE(n3.json_is_equal(&n1));
    n1.json_free();
    n2.json_free();
    n3.json_free();
}

TEST(Test_Json, test_swap) {
    json_node n1, n2;
    n1.json_init();
    n2.json_init();
    n1.json_set_string("Hello");
    n2.json_set_string("World!");
    n1.json_swap(&n2);
    EXPECT_EQ("World!", n1.json_get_string());
    EXPECT_EQ("Hello", n2.json_get_string());
    n1.json_free();
    n2.json_free();
}

TEST(Test_Json, test_access) {
    json_node n;

    /* test_access_null */
    n.json_init();
    n.json_set_string("a");
    n.json_free();
    EXPECT_EQ(JSONTYPE_NULL, n.json_get_type());
    n.json_free();

    /* test_access_boolean */
    n.json_init();
    n.json_set_string("a");
    n.json_set_bool(1);
    EXPECT_TRUE(n.json_get_bool());
    n.json_set_bool(0);
    EXPECT_FALSE(n.json_get_bool());
    n.json_free();

    /* test_access_number */
    n.json_init();
    n.json_set_string("a");
    n.json_set_number(1234.5);
    EXPECT_DOUBLE_EQ(1234.5, n.json_get_number());
    n.json_free();

    /* test_access_string */
    n.json_init();
    n.json_set_string("");
    EXPECT_EQ("", n.json_get_string());
    n.json_set_string("Hello");
    EXPECT_EQ("Hello", n.json_get_string());
    n.json_free();

    /* test_access_array */

    auto* a = new json_node();
    auto* e = new json_node();
    json_node* v;
    int i, j;
    a->json_init();
    a->json_set_array();
    for (i = 0; i < 10; i++) {
        v = new json_node();
        v->json_init();
        v->json_set_number(i);
        a->json_pushback_array_element(v);
    }

    EXPECT_EQ(10, a->json_get_array_size());
    for (i = 0; i < 10; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    a->json_popback_array_element();
    EXPECT_EQ(9, a->json_get_array_size());
    for (i = 0; i < 9; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(4, 0);
    EXPECT_EQ(9, a->json_get_array_size());
    for (i = 0; i < 9; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(8, 1);
    EXPECT_EQ(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    a->json_erase_array_element(0, 2);
    EXPECT_EQ(6, a->json_get_array_size());
    for (i = 0; i < 6; i++)
        EXPECT_DOUBLE_EQ((double) i + 2, a->json_get_array_index(i)->json_get_number());

    for (i = 0; i < 2; i++) {
        v = new json_node();
        v->json_init();
        v->json_set_number(i);
        a->json_insert_array_element(v, i);
    }

    EXPECT_EQ(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    EXPECT_EQ(8, a->json_get_array_size());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->json_get_array_index(i)->json_get_number());

    v = new json_node();
    v->json_init();
    e->json_set_string("Hello");
    a->json_pushback_array_element(v); /* Test if element is freed */

    a->json_clear_array();
    EXPECT_EQ(0, a->json_get_array_size());

    a->json_free();

    /* test_access_object */
    auto* o = new json_node();
    json_node* pn;
    int index;

    o->json_init();
    o->json_set_object();
    EXPECT_EQ(0, o->json_get_object_size());
    for (i = 0; i < 10; i++) {
        char key[2] = "a";
        key[0] += i;
        v = new json_node();
        v->json_init();
        v->json_set_number(i);
        o->json_set_object_value(key, v);
    }
    EXPECT_EQ(10, o->json_get_object_size());
    for (i = 0; i < 10; i++) {
        char key[] = "a";
        key[0] += i;
        index = o->json_find_object_index(key);
        EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
        pn = o->json_get_object_value(index);
        EXPECT_DOUBLE_EQ((double) i, pn->json_get_number());
    }

    index = o->json_find_object_index("j");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->json_remove_object_value(index);
    index = o->json_find_object_index("j");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ(9, o->json_get_object_size());

    index = o->json_find_object_index("a");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->json_remove_object_value(index);
    index = o->json_find_object_index("a");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ(8, o->json_get_object_size());

    for (i = 0; i < 8; i++) {
        char key[] = "a";
        key[0] += i + 1;
        EXPECT_DOUBLE_EQ((double) i + 1, o->json_get_object_value(o->json_find_object_index(key))->json_get_number());
    }

    v->json_set_string("Hello");
    o->json_set_object_value("World", v); /* Test if element is freed */

    pn = o->json_find_object_value("World");
    EXPECT_TRUE(pn != nullptr);
    EXPECT_EQ("Hello", pn->json_get_string());

    o->json_clear_object();
    EXPECT_EQ(0, o->json_get_object_size());

    o->json_free();
}


int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();//RUN_ALL_TESTS()运行所有测试案例
}