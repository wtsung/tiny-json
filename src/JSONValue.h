//
// Created by 吴伟聪 on 2021/5/27.
//

#ifndef MYJSON_OOP_JSONVALUE_H
#define MYJSON_OOP_JSONVALUE_H
#include <string>
#include <stack>
#include <vector>
#include <unordered_map>


//枚举Json类型：null,true,false,number,string,array,object
enum json_type{
    JSONTYPE_NULL,
    JSONTYPE_TRUE,
    JSONTYPE_FALSE,
    JSONTYPE_NUMBER,
    JSONTYPE_STRING,
    JSONTYPE_ARRAY,
    JSONTYPE_OBJECT
};
//json解析后的节点
struct json_node{
    json_type type; //节点类型
    double num;
    std::string str;
    std::vector<json_node*> arr;
    std::vector<std::pair<std::string, json_node*>> obj;
};

struct json_context{
    std::string s;
    const char* json;//json字符串
};

//枚举json解析返回值
enum {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE, //json空白错误
    JSON_PARSE_INVALID_VALUE, //非法值
    JSON_PARSE_NOT_SINGLE_VALUE, //非单个json
    JSON_PARSE_NUMBER_TOO_BIG, //数字过大
    JSON_PARSE_MISS_DoubleDuote, //缺失双引号
    JSON_PARSE_INVALID_STRING_ESCAPEVALUE, //非法转义符
    JSON_PARSE_INVALID_STRING_CHAR, //非法字符
    JSON_PARSE_INVALID_UNICODE_HEX, //非法unicode16进制符
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, //数组缺失逗号or中括号
    JSON_PARSE_MISS_COLON, //对象缺失冒号
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, //对象缺失逗号花括号
    JSON_PARSE_NOT_EXIST_KEY //对象key值不存在
};


int json_parse(json_node* node, const char* json);
json_type json_get_type(const json_node* node); //获得json节点类型

void json_free(json_node* n);
void json_init(json_node* node);

bool json_get_bool(const json_node* node);
void json_set_bool(json_node* node, bool b);

double json_get_number(const json_node* node);
void json_set_number(json_node* node, const double n);

void json_set_string(json_node* node, const std::string s);//设置json节点字符串
std::string json_get_string(const json_node* node);//获得json节点字符串
int json_get_string_length(const json_node* node);

int json_get_array_size(const json_node* node);
int json_get_array_capacity(const json_node* node);
json_node* json_get_array_index(const json_node* node, int index);
void json_erase_array_element(json_node* node, int index, int count);
void json_clear_array(json_node* node);
void json_pushback_array_element(json_node* node, json_node* value);
void json_popback_array_element(json_node* node);
void json_insert_array_element(json_node* node, json_node* v, int index);


int json_get_object_size(const json_node* n);
int json_get_object_capacity(const json_node* n);
const std::string json_get_object_key(const json_node* n, int index);
int json_get_object_key_length(const json_node* n, int index);
json_node* json_get_object_value(const json_node* n, int index);
void json_set_object_value(json_node* n, std::string key, json_node* v);
int json_find_object_index(const json_node* node, const std::string str);
json_node* json_find_object_value(json_node* node, const std::string str);
void json_clear_object(json_node* n);
void json_remove_object_value(json_node* node, int index);

std::string json_stringify(const json_node* node);

int json_is_equal(const json_node* lhs, const json_node* rhs);
void json_copy(json_node* dst, const json_node* src);
void json_move(json_node* dst, json_node* src);
void json_swap(json_node* lhs, json_node* rhs);

#endif //MYJSON_OOP_JSONVALUE_H
