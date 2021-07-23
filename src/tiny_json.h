#pragma once

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
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_NOT_SINGLE_VALUE,
    JSON_PARSE_NUMBER_TOO_BIG,
    JSON_PARSE_MISS_DOUBLEDUOTE,
    JSON_PARSE_INVALID_STRING_ESCAPEVALUE,
    JSON_PARSE_INVALID_STRING_CHAR,
    JSON_PARSE_INVALID_UNICODE_HEX,
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    JSON_PARSE_MISS_COLON,
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
    JSON_PARSE_NOT_EXIST_KEY
};

struct JsonContext {
    const char* json;
};

class JsonNode {
public:
    JsonNode() = default;
    JsonNode(const JsonNode& node);
    JsonNode& operator=(const JsonNode& node);
    ~JsonNode();

    int json_parse(const char* json);
    JsonType get_type() const;

    void json_free();
    void json_init();

    void set_null();
    bool get_bool() const;
    void set_bool(bool b);

    double get_number() const;
    void set_number(double num);

    void set_string(const std::string& str);
    std::string get_string() const;
    int get_string_length() const;

    void set_array();
    void set_array(const std::vector<JsonNode*>& arr);
    int get_array_size() const;
    JsonNode* get_array_index(int index) const;
    void erase_array_element(int index, int count);
    void clear_array();
    void pushback_array_element(JsonNode* node);
    void popback_array_element();
    void insert_array_element(JsonNode* node, int index);

    void set_object();
    void set_object(const std::vector<std::pair<std::string, JsonNode*>>& obj);
    int get_object_size() const;
    std::string get_object_key(int index) const;
    int get_object_key_length(int index) const;
    JsonNode* get_object_value(int index) const;
    void set_object_value(const std::string& key, JsonNode* node);
    int find_object_index(const std::string& str) const;
    JsonNode* find_object_value(const std::string& str);
    void clear_object();
    void remove_object_value(int index);
    void pushback_object_element(const std::string& key, JsonNode* node);

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
