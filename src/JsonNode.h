#include <string>
#include <vector>

//Json tpye：null,true,false,number,string,array,object
enum json_type {
    JSONTYPE_NULL,
    JSONTYPE_TRUE,
    JSONTYPE_FALSE,
    JSONTYPE_NUMBER,
    JSONTYPE_STRING,
    JSONTYPE_ARRAY,
    JSONTYPE_OBJECT
};

//Json parse return
enum {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,              //json空白
    JSON_PARSE_INVALID_VALUE,             //非法值
    JSON_PARSE_NOT_SINGLE_VALUE,          //非单个json
    JSON_PARSE_NUMBER_TOO_BIG,            //数字过大
    JSON_PARSE_MISS_DoubleDuote,          //缺失双引号
    JSON_PARSE_INVALID_STRING_ESCAPEVALUE,//非法转义符
    JSON_PARSE_INVALID_STRING_CHAR,       //非法字符
    JSON_PARSE_INVALID_UNICODE_HEX,       //非法unicode16进制符
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,//数组缺失逗号中括号
    JSON_PARSE_MISS_COLON,                  //对象缺失冒号
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, //对象缺失逗号花括号
    JSON_PARSE_NOT_EXIST_KEY                //对象key值不存在
};

struct json_context {
    const char* json;//json字符串
};

class json_node {
public:
    json_node() = default;
    json_node(const json_node& n);
    json_node& operator=(const json_node& n);
    ~json_node();

    int json_parse(const char* json);
    json_type json_get_type() const;

    void json_free();
    void json_init();

    void json_set_null();
    bool json_get_bool() const;
    void json_set_bool(bool b);

    double json_get_number() const;
    void json_set_number(double n);

    void json_set_string(std::string s);
    std::string json_get_string() const;
    int json_get_string_length() const;

    void json_set_array();
    void json_set_array(std::vector<json_node*> arr);
    int json_get_array_size() const;
    json_node* json_get_array_index(int index) const;
    void json_erase_array_element(int index, int count);
    void json_clear_array();
    void json_pushback_array_element(json_node* value);
    void json_popback_array_element();
    void json_insert_array_element(json_node* v, int index);

    void json_set_object();
    void json_set_object(std::vector<std::pair<std::string, json_node*>> obj);
    int json_get_object_size() const;
    std::string json_get_object_key(int index) const;
    int json_get_object_key_length(int index) const;
    json_node* json_get_object_value(int index) const;
    void json_set_object_value(std::string key, json_node* v);
    int json_find_object_index(std::string str) const;
    json_node* json_find_object_value(std::string str);
    void json_clear_object();
    void json_remove_object_value(int index);
    void json_pushback_object_element(const std::string& key, json_node* v);

    std::string json_stringify() const;

    int json_is_equal(json_node* rhs) const;
    void json_copy(const json_node* src);
    void json_move(json_node* src);
    void json_swap(json_node* rhs);

private:
    json_type type;
    double num;
    std::string str;
    std::vector<json_node*> arr;
    std::vector<std::pair<std::string, json_node*>> obj;
};
