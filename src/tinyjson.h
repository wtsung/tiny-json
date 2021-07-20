#include <string>
#include <vector>

//Json tpye：null,true,false,number,string,array,object
enum JsonType {
    JSON_TYPE_NULL,
    JSON_TYPE_TRUE,
    JSON_TYPE_FALSE,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
};

//Json parse return
enum {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,              //json空白
    JSON_PARSE_INVALID_VALUE,             //非法值
    JSON_PARSE_NOT_SINGLE_VALUE,          //非单个json
    JSON_PARSE_NUMBER_TOO_BIG,            //数字过大
    JSON_PARSE_MISS_DOUBLEDUOTE,          //缺失双引号
    JSON_PARSE_INVALID_STRING_ESCAPEVALUE,//非法转义符
    JSON_PARSE_INVALID_STRING_CHAR,       //非法字符
    JSON_PARSE_INVALID_UNICODE_HEX,       //非法unicode16进制符
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,//数组缺失逗号中括号
    JSON_PARSE_MISS_COLON,                  //对象缺失冒号
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, //对象缺失逗号花括号
    JSON_PARSE_NOT_EXIST_KEY                //对象key值不存在
};

struct JsonContext {
    const char* json;//json字符串
};

class JsonNode {
public:
    JsonNode() = default;
    JsonNode(const JsonNode& n);
    JsonNode& operator=(const JsonNode& n);
    ~JsonNode();

    int json_parse(const char* json);
    JsonType get_type() const;

    void json_free();
    void json_init();

    void set_null();
    bool get_bool() const;
    void set_bool(bool b);

    double get_number() const;
    void set_number(double n);

    void set_string(std::string s);
    std::string get_string() const;
    int get_string_length() const;

    void set_array();
    void set_array(std::vector<JsonNode*> arr);
    int get_array_size() const;
    JsonNode* get_array_index(int index) const;
    void erase_array_element(int index, int count);
    void clear_array();
    void pushback_array_element(JsonNode* value);
    void popback_array_element();
    void insert_array_element(JsonNode* v, int index);

    void set_object();
    void set_object(std::vector<std::pair<std::string, JsonNode*>> obj);
    int get_object_size() const;
    std::string get_object_key(int index) const;
    int get_object_key_length(int index) const;
    JsonNode* get_object_value(int index) const;
    void set_object_value(std::string key, JsonNode* v);
    int find_object_index(std::string str) const;
    JsonNode* find_object_value(std::string str);
    void clear_object();
    void remove_object_value(int index);
    void pushback_object_element(const std::string& key, JsonNode* v);

    std::string json_stringify() const;

    int json_is_equal(JsonNode* rhs) const;
    void json_copy(const JsonNode* src);
    void json_move(JsonNode* src);
    void json_swap(JsonNode* rhs);

private:
    JsonType type;
    double number;
    std::string string;
    std::vector<JsonNode*> array;
    std::vector<std::pair<std::string, JsonNode*>> object;
};
