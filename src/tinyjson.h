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

    int Parse(const char* json);
    JsonType GetType() const;

    void JsonFree();
    void JsonInit();

    void SetNull();
    bool GetBool() const;
    void SetBool(bool b);

    double GetNumber() const;
    void SetNumber(double n);

    void SetString(std::string s);
    std::string GetString() const;
    int GetStringLength() const;

    void SetArray();
    void SetArray(std::vector<JsonNode*> arr);
    int GetArraySize() const;
    JsonNode* GetArrayIndex(int index) const;
    void EraseArrayElement(int index, int count);
    void ClearArray();
    void PushbackArrayElement(JsonNode* value);
    void PopbackArrayElement();
    void InsertArrayElement(JsonNode* v, int index);

    void SetObject();
    void SetObject(std::vector<std::pair<std::string, JsonNode*>> obj);
    int GetObjectSize() const;
    std::string GetObjectKey(int index) const;
    int GetObjectKeyLength(int index) const;
    JsonNode* GetObjectValue(int index) const;
    void SetObjectValue(std::string key, JsonNode* v);
    int FindObjectIndex(std::string str) const;
    JsonNode* FindObjectValue(std::string str);
    void ClearObject();
    void RemoveObjectValue(int index);
    void PushbackObjectElement(const std::string& key, JsonNode* v);

    std::string JsonStringify() const;

    int IsEqual(JsonNode* rhs) const;
    void Copy(const JsonNode* src);
    void Move(JsonNode* src);
    void Swap(JsonNode* rhs);

private:
    JsonType type;
    double number;
    std::string string;
    std::vector<JsonNode*> array;
    std::vector<std::pair<std::string, JsonNode*>> object;
};
