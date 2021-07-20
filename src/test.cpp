#include "JsonNode.h"
#include <gtest/gtest.h>

TEST(TestJson, test_parse_null) {
    JsonNode v;
    v.JsonInit();
    v.SetBool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.Parse("null"));
    EXPECT_EQ(JSON_TYPE_NULL, v.GetType());
    v.JsonFree();
}

TEST(TestJson, test_parse_true) {
    JsonNode v;
    v.JsonInit();
    v.SetBool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.Parse("true"));
    EXPECT_EQ(JSON_TYPE_TRUE, v.GetType());
    v.JsonFree();
}

TEST(TestJson, test_parse_false) {
    JsonNode v;
    v.JsonInit();
    v.SetBool(false);
    EXPECT_EQ(JSON_PARSE_OK, v.Parse("false"));
    EXPECT_EQ(JSON_TYPE_FALSE, v.GetType());
    v.JsonFree();
}

#define TEST_PARSE_NUMBER(expect_num, json)                \
    do {                                                   \
        JsonNode n;                                       \
        n.JsonInit();                                     \
        n.SetBool(0);                                \
        EXPECT_EQ(JSON_PARSE_OK, n.Parse(json));      \
        EXPECT_EQ(JSON_TYPE_NUMBER, n.GetType());     \
        EXPECT_DOUBLE_EQ(expect_num, n.GetNumber()); \
        n.JsonFree();                                     \
    } while (0)

TEST(TestJson, test_parse_number) {
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
        JsonNode n;                                   \
        n.JsonInit();                                 \
        EXPECT_EQ(JSON_PARSE_OK, n.Parse(json));  \
        EXPECT_EQ(JSON_TYPE_STRING, n.GetType()); \
        EXPECT_EQ(expect_string, n.GetString()); \
        n.JsonFree();                                 \
    } while (0)

TEST(TestJson, test_parse_string) {
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

TEST(TestJson, test_parse_array) {
    JsonNode n;
    n.JsonInit();
    EXPECT_EQ(JSON_PARSE_OK, n.Parse("[ ]"));
    EXPECT_EQ(JSON_TYPE_ARRAY, n.GetType());
    EXPECT_EQ(0, n.GetArraySize());
    n.JsonFree();

    EXPECT_EQ(JSON_PARSE_OK, n.Parse("[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(JSON_TYPE_ARRAY, n.GetType());
    EXPECT_EQ(5, n.GetArraySize());
    EXPECT_EQ(JSON_TYPE_NULL, n.GetArrayIndex(0)->GetType());
    EXPECT_EQ(JSON_TYPE_FALSE, n.GetArrayIndex(1)->GetType());
    EXPECT_EQ(JSON_TYPE_TRUE, n.GetArrayIndex(2)->GetType());
    EXPECT_EQ(JSON_TYPE_NUMBER, n.GetArrayIndex(3)->GetType());
    EXPECT_EQ(JSON_TYPE_STRING, n.GetArrayIndex(4)->GetType());
    EXPECT_DOUBLE_EQ(123.0, n.GetArrayIndex(3)->GetNumber());
    EXPECT_EQ("abc", n.GetArrayIndex(4)->GetString());
    n.JsonFree();

    n.JsonInit();
    EXPECT_EQ(JSON_PARSE_OK, n.Parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(JSON_TYPE_ARRAY, n.GetType());
    EXPECT_EQ(4, n.GetArraySize());
    for (int i = 0; i < 4; i++) {
        JsonNode* a = n.GetArrayIndex(i);
        EXPECT_EQ(JSON_TYPE_ARRAY, a->GetType());
        EXPECT_EQ(i, a->GetArraySize());
        for (int j = 0; j < i; j++) {
            JsonNode* e = a->GetArrayIndex(j);
            EXPECT_EQ(JSON_TYPE_NUMBER, e->GetType());
            EXPECT_DOUBLE_EQ((double) j, e->GetNumber());
        }
    }
    n.JsonFree();
}

TEST(TestJson, test_parse_object) {
    JsonNode n;
    int i;

    n.JsonInit();
    EXPECT_EQ(JSON_PARSE_OK, n.Parse(" { } "));
    EXPECT_EQ(JSON_TYPE_OBJECT, n.GetType());
    EXPECT_EQ(0, n.GetObjectSize());
    n.JsonFree();

    n.JsonInit();
    EXPECT_EQ(JSON_PARSE_OK, n.Parse(
                                     " { "
                                     "\"n\" : null , "
                                     "\"f\" : false , "
                                     "\"t\" : true , "
                                     "\"i\" : 123 , "
                                     "\"s\" : \"abc\", "
                                     "\"a\" : [ 1, 2, 3 ],"
                                     "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                     " } "));
    EXPECT_EQ(JSON_TYPE_OBJECT, n.GetType());
    EXPECT_EQ(7, n.GetObjectSize());
    EXPECT_EQ("n", n.GetObjectKey(0));
    EXPECT_EQ(JSON_TYPE_NULL, n.GetObjectValue(0)->GetType());
    EXPECT_EQ("f", n.GetObjectKey(1));
    EXPECT_EQ(JSON_TYPE_FALSE, n.GetObjectValue(1)->GetType());
    EXPECT_EQ("t", n.GetObjectKey(2));
    EXPECT_EQ(JSON_TYPE_TRUE, n.GetObjectValue(2)->GetType());
    EXPECT_EQ("i", n.GetObjectKey(3));
    EXPECT_EQ(JSON_TYPE_NUMBER, n.GetObjectValue(3)->GetType());
    EXPECT_DOUBLE_EQ(123.0, n.GetObjectValue(3)->GetNumber());
    EXPECT_EQ("s", n.GetObjectKey(4));
    EXPECT_EQ(JSON_TYPE_STRING, n.GetObjectValue(4)->GetType());
    EXPECT_EQ("abc", n.GetObjectValue(4)->GetString());
    EXPECT_EQ("a", n.GetObjectKey(5));
    EXPECT_EQ(JSON_TYPE_ARRAY, n.GetObjectValue(5)->GetType());
    EXPECT_EQ(3, n.GetObjectValue(5)->GetArraySize());
    for (i = 0; i < 3; i++) {
        JsonNode* e = n.GetObjectValue(5)->GetArrayIndex(i);
        EXPECT_EQ(JSON_TYPE_NUMBER, e->GetType());
        EXPECT_DOUBLE_EQ(i + 1.0, e->GetNumber());
    }
    EXPECT_EQ("o", n.GetObjectKey(6));
    {
        JsonNode* o = n.GetObjectValue(6);
        EXPECT_EQ(JSON_TYPE_OBJECT, o->GetType());
        for (i = 0; i < 3; i++) {
            JsonNode* ov = o->GetObjectValue(i);
            EXPECT_TRUE('1' + i == o->GetObjectKey(i)[0]);
            EXPECT_EQ(1, o->GetObjectKeyLength(i));
            EXPECT_EQ(JSON_TYPE_NUMBER, ov->GetType());
            EXPECT_DOUBLE_EQ(i + 1.0, ov->GetNumber());
        }
    }
    n.JsonFree();
}


#define TEST_PARSE_ERROR(error, json)                \
    do {                                             \
        JsonNode n;                                 \
        n.JsonInit();                               \
        n.SetBool(0);                          \
        EXPECT_EQ(error, n.Parse(json));        \
        EXPECT_EQ(JSON_TYPE_NULL, n.GetType()); \
        n.JsonFree();                               \
    } while (0)

TEST(TestJson, test_parse_error) {
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
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DOUBLEDUOTE, "\"");
    TEST_PARSE_ERROR(JSON_PARSE_MISS_DOUBLEDUOTE, "\"abc");

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
        JsonNode n;                                  \
        std::string json2;                            \
        int length;                                   \
        n.JsonInit();                                \
        EXPECT_EQ(JSON_PARSE_OK, n.Parse(json)); \
        json2 = n.JsonStringify();                   \
        EXPECT_EQ(json, json2);                       \
        n.JsonFree();                                \
    } while (0)

TEST(TestJson, test_stringify) {
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
        JsonNode n1, n2;                               \
        n1.JsonInit();                                 \
        n2.JsonInit();                                 \
        EXPECT_EQ(JSON_PARSE_OK, n1.Parse(json1)); \
        EXPECT_EQ(JSON_PARSE_OK, n2.Parse(json2)); \
        EXPECT_EQ(equality, n1.IsEqual(&n2));     \
        n1.JsonFree();                                 \
        n2.JsonFree();                                 \
    } while (0)

TEST(TestJson, test_equal) {
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

TEST(TestJson, test_copy) {
    JsonNode n1, n2;
    n1.JsonInit();
    n1.Parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    n2.JsonInit();
    n2.Copy(&n1);
    EXPECT_TRUE(n2.IsEqual(&n1));
    n1.JsonFree();
    n2.JsonFree();
}

TEST(TestJson, test_move) {
    JsonNode n1, n2, n3;
    n1.JsonInit();
    n1.Parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    n2.JsonInit();
    n2.Copy(&n1);
    n3.JsonInit();
    n3.Move(&n2);
    EXPECT_EQ(JSON_TYPE_NULL, n2.GetType());
    EXPECT_TRUE(n3.IsEqual(&n1));
    n1.JsonFree();
    n2.JsonFree();
    n3.JsonFree();
}

TEST(TestJson, test_swap) {
    JsonNode n1, n2;
    n1.JsonInit();
    n2.JsonInit();
    n1.SetString("Hello");
    n2.SetString("World!");
    n1.Swap(&n2);
    EXPECT_EQ("World!", n1.GetString());
    EXPECT_EQ("Hello", n2.GetString());
    n1.JsonFree();
    n2.JsonFree();
}

TEST(TestJson, test_access) {
    JsonNode n;

    /* test_access_null */
    n.JsonInit();
    n.SetString("a");
    n.JsonFree();
    EXPECT_EQ(JSON_TYPE_NULL, n.GetType());
    n.JsonFree();

    /* test_access_boolean */
    n.JsonInit();
    n.SetString("a");
    n.SetBool(1);
    EXPECT_TRUE(n.GetBool());
    n.SetBool(0);
    EXPECT_FALSE(n.GetBool());
    n.JsonFree();

    /* test_access_number */
    n.JsonInit();
    n.SetString("a");
    n.SetNumber(1234.5);
    EXPECT_DOUBLE_EQ(1234.5, n.GetNumber());
    n.JsonFree();

    /* test_access_string */
    n.JsonInit();
    n.SetString("");
    EXPECT_EQ("", n.GetString());
    n.SetString("Hello");
    EXPECT_EQ("Hello", n.GetString());
    n.JsonFree();

    /* test_access_array */

    auto* a = new JsonNode();
    auto* e = new JsonNode();
    JsonNode* v;
    int i, j;
    a->JsonInit();
    a->SetArray();
    for (i = 0; i < 10; i++) {
        v = new JsonNode();
        v->JsonInit();
        v->SetNumber(i);
        a->PushbackArrayElement(v);
    }

    EXPECT_EQ(10, a->GetArraySize());
    for (i = 0; i < 10; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    a->PopbackArrayElement();
    EXPECT_EQ(9, a->GetArraySize());
    for (i = 0; i < 9; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    a->EraseArrayElement(4, 0);
    EXPECT_EQ(9, a->GetArraySize());
    for (i = 0; i < 9; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    a->EraseArrayElement(8, 1);
    EXPECT_EQ(8, a->GetArraySize());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    a->EraseArrayElement(0, 2);
    EXPECT_EQ(6, a->GetArraySize());
    for (i = 0; i < 6; i++)
        EXPECT_DOUBLE_EQ((double) i + 2, a->GetArrayIndex(i)->GetNumber());

    for (i = 0; i < 2; i++) {
        v = new JsonNode();
        v->JsonInit();
        v->SetNumber(i);
        a->InsertArrayElement(v, i);
    }

    EXPECT_EQ(8, a->GetArraySize());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    EXPECT_EQ(8, a->GetArraySize());
    for (i = 0; i < 8; i++)
        EXPECT_DOUBLE_EQ((double) i, a->GetArrayIndex(i)->GetNumber());

    v = new JsonNode();
    v->JsonInit();
    e->SetString("Hello");
    a->PushbackArrayElement(v); /* Test if element is freed */

    a->ClearArray();
    EXPECT_EQ(0, a->GetArraySize());

    a->JsonFree();

    /* test_access_object */
    auto* o = new JsonNode();
    JsonNode* pn;
    int index;

    o->JsonInit();
    o->SetObject();
    EXPECT_EQ(0, o->GetObjectSize());
    for (i = 0; i < 10; i++) {
        char key[2] = "a";
        key[0] += i;
        v = new JsonNode();
        v->JsonInit();
        v->SetNumber(i);
        o->SetObjectValue(key, v);
    }
    EXPECT_EQ(10, o->GetObjectSize());
    for (i = 0; i < 10; i++) {
        char key[] = "a";
        key[0] += i;
        index = o->FindObjectIndex(key);
        EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
        pn = o->GetObjectValue(index);
        EXPECT_DOUBLE_EQ((double) i, pn->GetNumber());
    }

    index = o->FindObjectIndex("j");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->RemoveObjectValue(index);
    index = o->FindObjectIndex("j");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ(9, o->GetObjectSize());

    index = o->FindObjectIndex("a");
    EXPECT_TRUE(index != JSON_PARSE_NOT_EXIST_KEY);
    o->RemoveObjectValue(index);
    index = o->FindObjectIndex("a");
    EXPECT_TRUE(index == JSON_PARSE_NOT_EXIST_KEY);
    EXPECT_EQ(8, o->GetObjectSize());

    for (i = 0; i < 8; i++) {
        char key[] = "a";
        key[0] += i + 1;
        EXPECT_DOUBLE_EQ((double) i + 1, o->GetObjectValue(o->FindObjectIndex(key))->GetNumber());
    }

    v->SetString("Hello");
    o->SetObjectValue("World", v); /* Test if element is freed */

    pn = o->FindObjectValue("World");
    EXPECT_TRUE(pn != nullptr);
    EXPECT_EQ("Hello", pn->GetString());

    o->ClearObject();
    EXPECT_EQ(0, o->GetObjectSize());

    o->JsonFree();
}


int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();//RUN_ALL_TESTS()运行所有测试案例
}