#include "tinyjson.h"
#include <cerrno>

JsonNode::JsonNode(const JsonNode& n) {
    this->JsonFree();
    this->Copy(&n);
}

JsonNode& JsonNode::operator=(const JsonNode& n) {
    this->JsonFree();
    this->Copy(&n);
    return *this;
}

JsonNode::~JsonNode() {
    this->JsonFree();
}

static void ParseWhitespace(JsonContext* c) {
    const char* str = c->json;
    while (*str == ' ' || *str == '\r' || *str == '\n' || *str == '\t')
        str++;
    c->json = str;
}

static int ParseNull(JsonNode* node, JsonContext* c) {
    assert(*c->json == 'n');
    if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l')
        return JSON_PARSE_INVALID_VALUE;
    node->SetNull();
    c->json += 4;
    return JSON_PARSE_OK;
}

static int ParseTrue(JsonNode* node, JsonContext* c) {
    assert(*c->json == 't');
    if (c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e') {
        node->SetBool(true);
        c->json += 4;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_EXPECT_VALUE;
    }
}

static int ParseFalse(JsonNode* node, JsonContext* c) {
    assert(*c->json == 'f');
    if (c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e') {
        node->SetBool(false);
        c->json += 5;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_INVALID_VALUE;
    }
}

#define ISDIGIT1TO9(ch) (((ch) >= '1') && ((ch) <= '9'))
#define ISDIGIT(ch) (((ch) >= '0') && ((ch) <= '9'))
static int ParseNumber(JsonNode* node, JsonContext* c) {
    const char* p = c->json;
    if (*p == '-') p++;//判断负号
    if (*p == '0') {   //判断0
        p++;
    } else {
        if (!ISDIGIT1TO9(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }

    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }

    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }
    errno = 0;
    double num_str = strtod(c->json, nullptr);//字符串转double
    //数字过大
    if (errno == ERANGE && (num_str == HUGE_VAL || num_str == -HUGE_VAL))
        return JSON_PARSE_NUMBER_TOO_BIG;
    node->SetNumber(num_str);
    c->json = p;
    return JSON_PARSE_OK;
}

//获取/u后的四位十六进制数
static const char* ParseHex4(const char* p, unsigned* u) {
    *u = 0;
    for (int i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9')
            *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            *u |= ch - 'A' + 10;
        else if (ch >= 'a' && ch <= 'f')
            *u |= ch - 'a' + 10;
        else
            return nullptr;
    }
    return p;
}

//utf-8解析压栈
static void EncodeUtf8(std::string& str, unsigned u) {
    if (u <= 0x7F)//此范围码点编码对应一个字节
        str.push_back(u & 0xFF);
    //或是加上前缀
    else if (u <= 0x7FF) {                      //此范围码点编码对应2个字节
        str.push_back(0xC0 | ((u >> 6) & 0xFF));//去掉后六位
        str.push_back(0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {                    //此范围码点编码对应3个字节
        str.push_back(0xE0 | ((u >> 12) & 0xFF));//去掉后12位
        str.push_back(0x80 | ((u >> 6) & 0x3F)); //去掉后6位
        str.push_back(0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);//此范围码点编码对应4个字节
        str.push_back(0xF0 | ((u >> 18) & 0xFF));
        str.push_back(0x80 | ((u >> 12) & 0x3F));
        str.push_back(0x80 | ((u >> 6) & 0x3F));
        str.push_back(0x80 | (u & 0x3F));
    }
}

static int ParseStringRaw(JsonContext* c, std::string& str) {
    assert(*c->json == '\"');
    unsigned u, u2;
    c->json++;
    const char* p = c->json;
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                c->json = p;
                return JSON_PARSE_OK;
            case '\0':
                str.clear();
                return JSON_PARSE_MISS_DOUBLEDUOTE;//缺失结尾双引号
            case '\\':
                switch (*p++) {
                    case '\"':
                        str.push_back('\"');
                        break;
                    case '\\':
                        str.push_back('\\');
                        break;
                    case 'n':
                        str.push_back('\n');
                        break;
                    case 'b':
                        str.push_back('\b');
                        break;
                    case 'f':
                        str.push_back('\f');
                        break;
                    case 'r':
                        str.push_back('\r');
                        break;
                    case 't':
                        str.push_back('\t');
                        break;
                    case '/':
                        str.push_back('/');
                        break;
                    case 'u':
                        if (!(p = ParseHex4(p, &u)))
                            return JSON_PARSE_INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair 代理对*///D800-DBFF此处为高代理对
                            if (*p++ != '\\')
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (*p++ != 'u')
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (!(p = ParseHex4(p, &u2)))
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (u2 < 0xDC00 || u2 > 0xDFFF)//DC00-DFFF为低代理对
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        EncodeUtf8(str, u);
                        break;
                    default:
                        str.clear();
                        return JSON_PARSE_INVALID_STRING_ESCAPEVALUE;
                }
                break;
            default:
                if ((unsigned char) ch < 0x20) {
                    str.clear();
                    return JSON_PARSE_INVALID_STRING_CHAR;//？？？
                }
                str.push_back(ch);
        }
    }
}

static int ParseString(JsonNode* node, JsonContext* c) {
    int ret;
    std::string s;
    if ((ret = ParseStringRaw(c, s)) == JSON_PARSE_OK) {
        node->SetString(s);
    }
    return ret;
}

void JsonNode::JsonFree() {
    switch (this->type) {
        case JSON_TYPE_ARRAY:
            for (auto node : this->array) {
                node->JsonFree();
                free(node);
            }
            this->array.clear();
            break;
        case JSON_TYPE_OBJECT:
            for (auto node : this->object) {
                node.second->JsonFree();
                free(node.second);
            }
            this->object.clear();
            break;
        default:
            break;
    }
    this->type = JSON_TYPE_NULL;
}

void JsonNode::JsonInit() {
    this->type = JSON_TYPE_NULL;
}

static int ParseValue(JsonNode* node, JsonContext* c);

static int ParseArray(JsonNode* node, JsonContext* c) {
    assert(*c->json == '[');
    node->SetArray();
    c->json++;
    ParseWhitespace(c);
    if (*c->json == ']') {
        node->SetArray({});
        c->json++;
        return JSON_PARSE_OK;
    }
    int ret;
    while (true) {
        auto n = new JsonNode();
        n->JsonInit();
        if ((ret = ParseValue(n, c)) != JSON_PARSE_OK) {
            break;
        }
        node->PushbackArrayElement(n);
        ParseWhitespace(c);
        if (*c->json == ',') {
            c->json++;
            ParseWhitespace(c);
        } else if (*c->json == ']') {
            node->SetArray();
            c->json++;
            return JSON_PARSE_OK;
        } else {
            for (int i = 0; i < node->GetArraySize(); i++)
                node->GetArrayIndex(i)->JsonFree();
            node->JsonFree();
            return JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
    for (int i = 0; i < node->GetArraySize(); i++)
        node->GetArrayIndex(i)->JsonFree();
    node->JsonFree();
    return ret;
}

static int ParseObject(JsonNode* node, JsonContext* c) {
    int ret;
    assert(*c->json == '{');
    node->SetObject();
    c->json++;
    std::string str;
    JsonNode* n;
    ParseWhitespace(c);
    if (*c->json == '}') {
        c->json++;
        node->SetObject({});
        return JSON_PARSE_OK;
    }
    while (true) {
        n = new JsonNode();
        str.clear();
        if (*c->json != '"') {
            ret = JSON_PARSE_NOT_EXIST_KEY;
            break;
        }
        if ((ret = ParseStringRaw(c, str)) != JSON_PARSE_OK)
            break;
        ParseWhitespace(c);
        if (*c->json != ':') {
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        ParseWhitespace(c);
        if ((ret = ParseValue(n, c)) != JSON_PARSE_OK)
            break;
        node->PushbackObjectElement(str, n);
        ParseWhitespace(c);
        if (*c->json == ',') {
            c->json++;
            ParseWhitespace(c);
        } else if (*c->json == '}') {
            c->json++;
            node->SetObject();
            return JSON_PARSE_OK;
        } else {
            ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    for (int i = 0; i < node->GetObjectSize(); i++) {
        node->GetObjectValue(i)->JsonFree();
    }
    node->JsonFree();
    n->JsonFree();
    return ret;
}

static int ParseValue(JsonNode* node, JsonContext* c) {
    switch (*(c->json)) {
        case '\0':
            return JSON_PARSE_EXPECT_VALUE;
        case 'n':
            return ParseNull(node, c);
        case 't':
            return ParseTrue(node, c);
        case 'f':
            return ParseFalse(node, c);
        case '\"':
            return ParseString(node, c);
        case '[':
            return ParseArray(node, c);
        case '{':
            return ParseObject(node, c);
        default:
            return ParseNumber(node, c);
    }
}


int JsonNode::Parse(const char* json) {
    JsonContext c{};
    c.json = json;
    this->JsonInit();
    ParseWhitespace(&c);
    int ret;
    if ((ret = ParseValue(this, &c)) == JSON_PARSE_OK) {
        ParseWhitespace(&c);
        if (*(c.json) != '\0') {
            this->SetNull();
            ret = JSON_PARSE_NOT_SINGLE_VALUE;
        }
    }
    return ret;
}

JsonType JsonNode::GetType() const {
    return this->type;
}

void JsonNode::SetNull() {
    this->JsonFree();
    this->type = JSON_TYPE_NULL;
}

bool JsonNode::GetBool() const {
    assert(this->type == JSON_TYPE_TRUE || this->type == JSON_TYPE_FALSE);
    return this->type == JSON_TYPE_TRUE;
}

void JsonNode::SetBool(bool b) {
    this->JsonFree();
    b ? this->type = JSON_TYPE_TRUE : this->type = JSON_TYPE_FALSE;
}


double JsonNode::GetNumber() const {
    assert(this->type == JSON_TYPE_NUMBER);
    return this->number;
}
void JsonNode::SetNumber(double n) {
    this->type = JSON_TYPE_NUMBER;
    this->number = n;
}

void JsonNode::SetString(std::string s) {
    this->string = s;
    this->type = JSON_TYPE_STRING;
}
std::string JsonNode::GetString() const {
    assert(this->type == JSON_TYPE_STRING);
    return this->string;
}
int JsonNode::GetStringLength() const {
    assert(this->type == JSON_TYPE_STRING);
    return this->string.size();
}

void JsonNode::SetArray() {
    this->type = JSON_TYPE_ARRAY;
}

void JsonNode::SetArray(std::vector<JsonNode*> a) {
    this->array = a;
    this->type = JSON_TYPE_ARRAY;
}

int JsonNode::GetArraySize() const {
    assert(this->type == JSON_TYPE_ARRAY);
    return this->array.size();
}

JsonNode* JsonNode::GetArrayIndex(int index) const {
    assert(this->type == JSON_TYPE_ARRAY);
    if (index < 0 || index >= this->array.size())
        return nullptr;
    return this->array[index];
}

void JsonNode::EraseArrayElement(int index, int count) {
    assert(this->type == JSON_TYPE_ARRAY);
    auto iter_b = this->array.begin();
    int i = 0;
    int j = 0;
    if (count <= 0)
        return;
    while (i++ < index)
        iter_b++;
    auto iter_e = iter_b;
    while (j++ < count)
        iter_e++;
    this->array.erase(iter_b, iter_e);
}

void JsonNode::PushbackArrayElement(JsonNode* v) {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array.push_back(v);
}

void JsonNode::PopbackArrayElement() {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array[this->array.size() - 1]->JsonFree();
    this->array.pop_back();
}

void JsonNode::InsertArrayElement(JsonNode* v, int index) {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array.insert(this->array.begin() + index, v);
}

void JsonNode::ClearArray() {
    assert(this->type == JSON_TYPE_ARRAY);
    for (auto node : this->array)
        node->JsonFree();
    this->array.clear();
}

void JsonNode::SetObject() {
    this->type = JSON_TYPE_OBJECT;
}

void JsonNode::SetObject(std::vector<std::pair<std::string, JsonNode*>> o) {
    this->object = o;
    this->type = JSON_TYPE_OBJECT;
}

int JsonNode::GetObjectSize() const {
    assert(this->type == JSON_TYPE_OBJECT);
    return this->object.size();
}

std::string JsonNode::GetObjectKey(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index)
        iter++;
    return iter->first;
}
int JsonNode::GetObjectKeyLength(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index)
        iter++;
    return iter->first.size();
}

JsonNode* JsonNode::GetObjectValue(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index)
        iter++;
    return iter->second;
}

void JsonNode::SetObjectValue(std::string key, JsonNode* v) {
    assert(this->type == JSON_TYPE_OBJECT);
    auto iter = this->object.begin();
    while (iter != this->object.end()) {
        if (iter->first == key) {
            iter->second->JsonFree();
            iter->second = v;
            return;
        }
        iter++;
    }
    this->object.emplace_back(std::pair<std::string, JsonNode*>(key, v));
}

int JsonNode::FindObjectIndex(const std::string str) const {
    int index = 0;
    assert(this->type == JSON_TYPE_OBJECT);
    for (auto iter : this->object) {
        if (iter.first == str)
            return index;
        index++;
    }
    return JSON_PARSE_NOT_EXIST_KEY;
}

JsonNode* JsonNode::FindObjectValue(const std::string str) {
    assert(this->type == JSON_TYPE_OBJECT);
    for (const auto& node : this->object) {
        if (node.first == str)
            return node.second;
    }
    return nullptr;
}

void JsonNode::ClearObject() {
    assert(this->type == JSON_TYPE_OBJECT);
    this->JsonFree();
    this->object.clear();
    this->type = JSON_TYPE_OBJECT;
}

void JsonNode::RemoveObjectValue(int index) {
    assert(this->type == JSON_TYPE_OBJECT);
    auto iter = this->object.begin();
    int i = 0;
    while (i++ < index)
        iter++;
    iter->second->JsonFree();
    this->object.erase(iter);
}

void JsonNode::PushbackObjectElement(const std::string& key, JsonNode* v) {
    assert(this->type == JSON_TYPE_OBJECT);
    this->object.emplace_back(std::pair<std::string, JsonNode*>(key, v));
}

static void JsonStringify_string(const JsonNode* node, std::string& str) {
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    assert(node->GetType() == JSON_TYPE_STRING);
    str.push_back('"');
    for (int i = 0; i < node->GetStringLength(); i++) {
        char ch = node->GetString()[i];
        switch (ch) {
            case '\"':
                str += '\\';
                str += '\"';
                break;
            case '\\':
                str += '\\';
                str += '\\';
                break;
            case '\b':
                str += '\\';
                str += 'b';
                break;
            case '\f':
                str += '\\';
                str += 'f';
                break;
            case '\n':
                str += '\\';
                str += 'n';
                break;
            case '\r':
                str += '\\';
                str += 'r';
                break;
            case '\t':
                str += '\\';
                str += 't';
                break;
            default:
                if (ch < 0x20) {
                    str += '\\';
                    str += 'u';
                    str += '0';
                    str += '0';
                    str += hex_digits[ch >> 4];
                    str += hex_digits[ch & 15];
                } else
                    str += node->GetString()[i];
        }
    }
    str.push_back('"');
}
static void JsonStringify_number(const JsonNode* node, std::string& str) {
    char tmp[32];
    sprintf(tmp, "%.17g", node->GetNumber());
    str += tmp;
}

static void JsonStringify_value(const JsonNode* node, std::string& str) {
    switch (node->GetType()) {
        case JSON_TYPE_NULL:
            str += "null";
            break;
        case JSON_TYPE_TRUE:
            str += "true";
            break;
        case JSON_TYPE_FALSE:
            str += "false";
            break;
        case JSON_TYPE_NUMBER:
            JsonStringify_number(node, str);
            break;
        case JSON_TYPE_STRING:
            JsonStringify_string(node, str);
            break;
        case JSON_TYPE_ARRAY:
            str += '[';
            for (int i = 0; i < node->GetArraySize(); i++) {
                if (i > 0)
                    str += ',';
                JsonStringify_value(node->GetArrayIndex(i), str);
            }
            str += ']';
            break;
        case JSON_TYPE_OBJECT:
            str += '{';
            for (int i = 0; i < node->GetObjectSize(); i++) {
                if (i > 0)
                    str += ',';
                str += '"';
                str += node->GetObjectKey(i);
                str += '"';
                str += ':';
                JsonStringify_value(node->GetObjectValue(i), str);
            }
            str += '}';
            break;
    }
}
std::string JsonNode::JsonStringify() const {
    std::string s;
    JsonStringify_value(this, s);
    return s;
}

int JsonNode::IsEqual(JsonNode* rhs) const {
    assert(rhs != nullptr);
    if (this->type != rhs->type)
        return 0;
    auto iter = this->object.begin();
    auto iter_rhs = rhs->object.begin();
    switch (this->type) {
        case JSON_TYPE_STRING:
            return this->string == rhs->string;
        case JSON_TYPE_NUMBER:
            return this->number == rhs->number;
        case JSON_TYPE_ARRAY:
            if (this->array.size() != rhs->array.size())
                return 0;
            for (int i = 0; i < this->array.size(); i++)
                if (!this->array[i]->IsEqual(rhs->array[i]))
                    return 0;
            return 1;
        case JSON_TYPE_OBJECT:
            if (this->object.size() != rhs->object.size())
                return 0;
            int index;
            for (; iter != this->object.end(); iter++) {
                index = rhs->FindObjectIndex(iter->first);
                if (!iter->second->IsEqual(rhs->object[index].second))
                    return 0;
            }
            return 1;
        default:
            return 1;
    }
}

void JsonNode::Copy(const JsonNode* src) {
    int i;
    assert(src != this);
    auto iter = src->object.begin();
    switch (src->type) {
        case JSON_TYPE_STRING:
            this->SetString(src->string);
            break;
        case JSON_TYPE_ARRAY:
            this->type = JSON_TYPE_ARRAY;
            JsonNode* tmp_array;
            for (i = 0; i < src->array.size(); i++) {
                tmp_array = new JsonNode();
                memcpy(tmp_array, src->array[i], sizeof(JsonNode));
                this->PushbackArrayElement(tmp_array);
            }
            break;
        case JSON_TYPE_OBJECT:
            this->type = JSON_TYPE_OBJECT;
            JsonNode* tmp_obj;
            for (; iter != src->object.end(); iter++) {
                tmp_obj = new JsonNode();
                tmp_obj->Copy(iter->second);
                this->PushbackObjectElement(iter->first, tmp_obj);
            }
            break;
        default:
            this->JsonFree();
            memcpy(this, src, sizeof(JsonNode));
            break;
    }
}

void JsonNode::Move(JsonNode* src) {
    assert(src != nullptr && src != this);
    this->JsonFree();
    this->Copy(src);
    src->JsonInit();
}

void JsonNode::Swap(JsonNode* rhs) {
    assert(rhs != nullptr);
    if (this != rhs) {
        auto* tmp = new JsonNode();
        tmp->Move(this);
        this->Move(rhs);
        rhs->Move(tmp);
        tmp->JsonFree();
    }
}