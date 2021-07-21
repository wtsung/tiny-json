#include "tinyjson.h"
#include <cerrno>

JsonNode::JsonNode(const JsonNode& node) {
    this->json_free();
    this->json_copy(&node);
}

JsonNode& JsonNode::operator=(const JsonNode& node) {
    this->json_free();
    this->json_copy(&node);
    return *this;
}

JsonNode::~JsonNode() {
    this->json_free();
}

static void parse_whitespace(JsonContext* ctx) {
    const char* str = ctx->json;
    while (*str == ' ' || *str == '\r' || *str == '\n' || *str == '\t') {
        str++;
    }
    ctx->json = str;
}

static int parse_null(JsonNode* node, JsonContext* ctx) {
    assert(*ctx->json == 'n');
    if (ctx->json[1] != 'u' || ctx->json[2] != 'l' || ctx->json[3] != 'l') {
        return JSON_PARSE_INVALID_VALUE;
    }
    node->set_null();
    ctx->json += 4;
    return JSON_PARSE_OK;
}

static int parse_true(JsonNode* node, JsonContext* ctx) {
    assert(*ctx->json == 't');
    if (ctx->json[1] == 'r' && ctx->json[2] == 'u' && ctx->json[3] == 'e') {
        node->set_bool(true);
        ctx->json += 4;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_EXPECT_VALUE;
    }
}

static int parse_false(JsonNode* node, JsonContext* ctx) {
    assert(*ctx->json == 'f');
    if (ctx->json[1] == 'a' && ctx->json[2] == 'l' && ctx->json[3] == 's' && ctx->json[4] == 'e') {
        node->set_bool(false);
        ctx->json += 5;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_INVALID_VALUE;
    }
}

#define ISDIGIT1TO9(ch) (((ch) >= '1') && ((ch) <= '9'))
#define ISDIGIT(ch) (((ch) >= '0') && ((ch) <= '9'))
static int parse_number(JsonNode* node, JsonContext* ctx) {
    const char* p = ctx->json;
    if (*p == '-') {
        p++;
    }
    if (*p == '0') {
        p++;
    } else {
        if (!ISDIGIT1TO9(*p)) {
            return JSON_PARSE_INVALID_VALUE;
        }
        while (ISDIGIT(*p)) {
            p++;
        }
    }

    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) {
            return JSON_PARSE_INVALID_VALUE;
        }
        while (ISDIGIT(*p)) {
            p++;
        }
    }

    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') {
            p++;
        }
        if (!ISDIGIT(*p)) {
            return JSON_PARSE_INVALID_VALUE;
        }
        while (ISDIGIT(*p)) {
            p++;
        }
    }
    errno = 0;
    double num_str = strtod(ctx->json, nullptr);//字符串转double
    if (errno == ERANGE && (num_str == HUGE_VAL || num_str == -HUGE_VAL)) {
        return JSON_PARSE_NUMBER_TOO_BIG;
    }
    node->set_number(num_str);
    ctx->json = p;
    return JSON_PARSE_OK;
}

//获取/u后的四位十六进制数
static const char* parse_hex4(const char* p, unsigned* u) {
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
static void encode_utf8(std::string& str, unsigned u) {
    if (u <= 0x7F) {
        str.push_back(u & 0xFF);
    }
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

static int parse_string_raw(JsonContext* ctx, std::string& str) {
    assert(*ctx->json == '\"');
    unsigned u, u2;
    ctx->json++;
    const char* p = ctx->json;
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                ctx->json = p;
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
                        if (!(p = parse_hex4(p, &u))) {
                            return JSON_PARSE_INVALID_UNICODE_HEX;
                        }
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair 代理对*///D800-DBFF此处为高代理对
                            if (*p++ != '\\') {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (*p++ != 'u') {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (!(p = parse_hex4(p, &u2))) {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (u2 < 0xDC00 || u2 > 0xDFFF) {//DC00-DFFF为低代理对
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(str, u);
                        break;
                    default:
                        str.clear();
                        return JSON_PARSE_INVALID_STRING_ESCAPEVALUE;
                }
                break;
            default:
                if ((unsigned char) ch < 0x20) {
                    str.clear();
                    return JSON_PARSE_INVALID_STRING_CHAR;
                }
                str.push_back(ch);
        }
    }
}

static int parse_string(JsonNode* node, JsonContext* ctx) {
    int ret;
    std::string s;
    if ((ret = parse_string_raw(ctx, s)) == JSON_PARSE_OK) {
        node->set_string(s);
    }
    return ret;
}

void JsonNode::json_free() {
    switch (this->type) {
        case JSON_TYPE_ARRAY:
            for (auto node : this->array) {
                node->json_free();
                free(node);
            }
            this->array.clear();
            break;
        case JSON_TYPE_OBJECT:
            for (auto node : this->object) {
                node.second->json_free();
                free(node.second);
            }
            this->object.clear();
            break;
        default:
            break;
    }
    this->type = JSON_TYPE_NULL;
}

void JsonNode::json_init() {
    this->type = JSON_TYPE_NULL;
}

static int parse_value(JsonNode* node, JsonContext* ctx);

static int parse_array(JsonNode* node, JsonContext* ctx) {
    assert(*ctx->json == '[');
    node->set_array();
    ctx->json++;
    parse_whitespace(ctx);
    if (*ctx->json == ']') {
        node->set_array({});
        ctx->json++;
        return JSON_PARSE_OK;
    }
    int ret;
    while (true) {
        auto n = new JsonNode();
        n->json_init();
        if ((ret = parse_value(n, ctx)) != JSON_PARSE_OK) {
            break;
        }
        node->pushback_array_element(n);
        parse_whitespace(ctx);
        if (*ctx->json == ',') {
            ctx->json++;
            parse_whitespace(ctx);
        } else if (*ctx->json == ']') {
            node->set_array();
            ctx->json++;
            return JSON_PARSE_OK;
        } else {
            for (int i = 0; i < node->get_array_size(); i++) {
                node->get_array_index(i)->json_free();
            }
            node->json_free();
            return JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
    for (int i = 0; i < node->get_array_size(); i++) {
        node->get_array_index(i)->json_free();
    }
    node->json_free();
    return ret;
}

static int parse_object(JsonNode* node, JsonContext* ctx) {
    int ret;
    assert(*ctx->json == '{');
    node->set_object();
    ctx->json++;
    std::string str;
    JsonNode* n;
    parse_whitespace(ctx);
    if (*ctx->json == '}') {
        ctx->json++;
        node->set_object();
        return JSON_PARSE_OK;
    }
    while (true) {
        n = new JsonNode();
        str.clear();
        if (*ctx->json != '"') {
            ret = JSON_PARSE_NOT_EXIST_KEY;
            break;
        }
        if ((ret = parse_string_raw(ctx, str)) != JSON_PARSE_OK) {
            break;
        }
        parse_whitespace(ctx);
        if (*ctx->json != ':') {
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        ctx->json++;
        parse_whitespace(ctx);
        if ((ret = parse_value(n, ctx)) != JSON_PARSE_OK) {
            break;
        }
        node->pushback_object_element(str, n);
        parse_whitespace(ctx);
        if (*ctx->json == ',') {
            ctx->json++;
            parse_whitespace(ctx);
        } else if (*ctx->json == '}') {
            ctx->json++;
            node->set_object();
            return JSON_PARSE_OK;
        } else {
            ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    for (int i = 0; i < node->get_object_size(); i++) {
        node->get_object_value(i)->json_free();
    }
    node->json_free();
    n->json_free();
    return ret;
}

static int parse_value(JsonNode* node, JsonContext* ctx) {
    switch (*(ctx->json)) {
        case '\0':
            return JSON_PARSE_EXPECT_VALUE;
        case 'n':
            return parse_null(node, ctx);
        case 't':
            return parse_true(node, ctx);
        case 'f':
            return parse_false(node, ctx);
        case '\"':
            return parse_string(node, ctx);
        case '[':
            return parse_array(node, ctx);
        case '{':
            return parse_object(node, ctx);
        default:
            return parse_number(node, ctx);
    }
}


int JsonNode::json_parse(const char* json) {
    JsonContext ctx{};
    ctx.json = json;
    this->json_init();
    parse_whitespace(&ctx);
    int ret;
    if ((ret = parse_value(this, &ctx)) == JSON_PARSE_OK) {
        parse_whitespace(&ctx);
        if (*(ctx.json) != '\0') {
            this->set_null();
            ret = JSON_PARSE_NOT_SINGLE_VALUE;
        }
    }
    return ret;
}

JsonType JsonNode::get_type() const {
    return this->type;
}

void JsonNode::set_null() {
    this->json_free();
    this->type = JSON_TYPE_NULL;
}

bool JsonNode::get_bool() const {
    assert(this->type == JSON_TYPE_TRUE || this->type == JSON_TYPE_FALSE);
    return this->type == JSON_TYPE_TRUE;
}

void JsonNode::set_bool(bool b) {
    this->json_free();
    b ? this->type = JSON_TYPE_TRUE : this->type = JSON_TYPE_FALSE;
}

double JsonNode::get_number() const {
    assert(this->type == JSON_TYPE_NUMBER);
    return this->number;
}
void JsonNode::set_number(double num) {
    this->type = JSON_TYPE_NUMBER;
    this->number = num;
}

void JsonNode::set_string(const std::string& str) {
    this->string = str;
    this->type = JSON_TYPE_STRING;
}
std::string JsonNode::get_string() const {
    assert(this->type == JSON_TYPE_STRING);
    return this->string;
}
int JsonNode::get_string_length() const {
    assert(this->type == JSON_TYPE_STRING);
    return this->string.size();
}

void JsonNode::set_array() {
    this->type = JSON_TYPE_ARRAY;
}

void JsonNode::set_array(const std::vector<JsonNode*>& arr) {
    JsonNode* node_tmp;
    for (auto node : arr) {
        node_tmp = new JsonNode();
        node_tmp->json_copy(node);
        this->array.emplace_back(node_tmp);
    }
    this->type = JSON_TYPE_ARRAY;
}

int JsonNode::get_array_size() const {
    assert(this->type == JSON_TYPE_ARRAY);
    return this->array.size();
}

JsonNode* JsonNode::get_array_index(int index) const {
    assert(this->type == JSON_TYPE_ARRAY);
    if (index < 0 || index >= this->array.size()) {
        return nullptr;
    }
    return this->array[index];
}

void JsonNode::erase_array_element(int index, int count) {
    assert(this->type == JSON_TYPE_ARRAY);
    auto iter_b = this->array.begin();
    int i = 0;
    int j = 0;
    if (count <= 0) {
        return;
    }
    while (i++ < index) {
        iter_b++;
    }
    auto iter_e = iter_b;
    while (j++ < count) {
        iter_e++;
    }
    this->array.erase(iter_b, iter_e);
}

void JsonNode::pushback_array_element(JsonNode* node) {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array.push_back(node);
}

void JsonNode::popback_array_element() {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array[this->array.size() - 1]->json_free();
    this->array.pop_back();
}

void JsonNode::insert_array_element(JsonNode* node, int index) {
    assert(this->type == JSON_TYPE_ARRAY);
    this->array.insert(this->array.begin() + index, node);
}

void JsonNode::clear_array() {
    assert(this->type == JSON_TYPE_ARRAY);
    for (auto node : this->array) {
        node->json_free();
    }
    this->array.clear();
}

void JsonNode::set_object() {
    this->type = JSON_TYPE_OBJECT;
}

void JsonNode::set_object(const std::vector<std::pair<std::string, JsonNode*>>& obj) {
    JsonNode* node_tmp;
    for (auto node : obj) {
        node_tmp = new JsonNode();
        node_tmp->json_copy(node.second);
        this->object.emplace_back(std::pair<std::string, JsonNode*>(node.first,node_tmp));
    }
    this->type = JSON_TYPE_OBJECT;
}

int JsonNode::get_object_size() const {
    assert(this->type == JSON_TYPE_OBJECT);
    return this->object.size();
}

std::string JsonNode::get_object_key(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index) {
        iter++;
    }
    return iter->first;
}
int JsonNode::get_object_key_length(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index) {
        iter++;
    }
    return iter->first.size();
}

JsonNode* JsonNode::get_object_value(int index) const {
    assert(this->type == JSON_TYPE_OBJECT && index >= 0 && index < this->object.size());
    int i = 0;
    auto iter = this->object.begin();
    while (i++ < index) {
        iter++;
    }
    return iter->second;
}

void JsonNode::set_object_value(const std::string& key, JsonNode* node) {
    assert(this->type == JSON_TYPE_OBJECT);
    auto iter = this->object.begin();
    while (iter != this->object.end()) {
        if (iter->first == key) {
            iter->second->json_free();
            iter->second = node;
            return;
        }
        iter++;
    }
    this->object.emplace_back(std::pair<std::string, JsonNode*>(key, node));
}

int JsonNode::find_object_index(const std::string& str) const {
    int index = 0;
    assert(this->type == JSON_TYPE_OBJECT);
    for (auto iter : this->object) {
        if (iter.first == str) {
            return index;
        }
        index++;
    }
    return JSON_PARSE_NOT_EXIST_KEY;
}

JsonNode* JsonNode::find_object_value(const std::string& str) {
    assert(this->type == JSON_TYPE_OBJECT);
    for (const auto& node : this->object) {
        if (node.first == str) {
            return node.second;
        }
    }
    return nullptr;
}

void JsonNode::clear_object() {
    assert(this->type == JSON_TYPE_OBJECT);
    this->json_free();
    this->object.clear();
    this->type = JSON_TYPE_OBJECT;
}

void JsonNode::remove_object_value(int index) {
    assert(this->type == JSON_TYPE_OBJECT);
    auto iter = this->object.begin();
    int i = 0;
    while (i++ < index) {
        iter++;
    }
    iter->second->json_free();
    this->object.erase(iter);
}

void JsonNode::pushback_object_element(const std::string& key, JsonNode* node) {
    assert(this->type == JSON_TYPE_OBJECT);
    this->object.emplace_back(std::pair<std::string, JsonNode*>(key, node));
}

static void JsonStringify_string(const JsonNode* node, std::string& str) {
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    assert(node->get_type() == JSON_TYPE_STRING);
    str.push_back('"');
    for (int i = 0; i < node->get_string_length(); i++) {
        char ch = node->get_string()[i];
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
                    str += node->get_string()[i];
        }
    }
    str.push_back('"');
}
static void JsonStringify_number(const JsonNode* node, std::string& str) {
    char tmp[32];
    sprintf(tmp, "%.17g", node->get_number());
    str += tmp;
}

static void JsonStringify_value(const JsonNode* node, std::string& str) {
    switch (node->get_type()) {
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
            for (int i = 0; i < node->get_array_size(); i++) {
                if (i > 0) {
                    str += ',';
                }
                JsonStringify_value(node->get_array_index(i), str);
            }
            str += ']';
            break;
        case JSON_TYPE_OBJECT:
            str += '{';
            for (int i = 0; i < node->get_object_size(); i++) {
                if (i > 0) {
                    str += ',';
                }
                str += '"';
                str += node->get_object_key(i);
                str += '"';
                str += ':';
                JsonStringify_value(node->get_object_value(i), str);
            }
            str += '}';
            break;
    }
}
std::string JsonNode::json_stringify() const {
    std::string s;
    JsonStringify_value(this, s);
    return s;
}

int JsonNode::json_is_equal(JsonNode* rhs) const {
    assert(rhs != nullptr);
    if (this->type != rhs->type) {
        return 0;
    }
    auto iter = this->object.begin();
    auto iter_rhs = rhs->object.begin();
    switch (this->type) {
        case JSON_TYPE_STRING:
            return this->string == rhs->string;
        case JSON_TYPE_NUMBER:
            return this->number == rhs->number;
        case JSON_TYPE_ARRAY:
            if (this->array.size() != rhs->array.size()) {
                return 0;
            }
            for (int i = 0; i < this->array.size(); i++)
                if (!this->array[i]->json_is_equal(rhs->array[i])) {
                    return 0;
                }
            return 1;
        case JSON_TYPE_OBJECT:
            if (this->object.size() != rhs->object.size()) {
                return 0;
            }
            int index;
            for (; iter != this->object.end(); iter++) {
                index = rhs->find_object_index(iter->first);
                if (!iter->second->json_is_equal(rhs->object[index].second)) {
                    return 0;
                }
            }
            return 1;
        default:
            return 1;
    }
}

void JsonNode::json_copy(const JsonNode* src) {
    int i;
    assert(src != this);
    auto iter = src->object.begin();
    switch (src->type) {
        case JSON_TYPE_STRING:
            this->set_string(src->string);
            break;
        case JSON_TYPE_ARRAY:
            this->type = JSON_TYPE_ARRAY;
            JsonNode* tmp_array;
            for (i = 0; i < src->array.size(); i++) {
                tmp_array = new JsonNode();
                memcpy(tmp_array, src->array[i], sizeof(JsonNode));
                this->pushback_array_element(tmp_array);
            }
            break;
        case JSON_TYPE_OBJECT:
            this->type = JSON_TYPE_OBJECT;
            JsonNode* tmp_obj;
            for (; iter != src->object.end(); iter++) {
                tmp_obj = new JsonNode();
                tmp_obj->json_copy(iter->second);
                this->pushback_object_element(iter->first, tmp_obj);
            }
            break;
        default:
            this->json_free();
            memcpy(this, src, sizeof(JsonNode));
            break;
    }
}

void JsonNode::json_move(JsonNode* src) {
    assert(src != nullptr && src != this);
    this->json_free();
    this->json_copy(src);
    src->json_init();
}

void JsonNode::json_swap(JsonNode* rhs) {
    assert(rhs != nullptr);
    if (this != rhs) {
        auto* tmp = new JsonNode();
        tmp->json_move(this);
        this->json_move(rhs);
        rhs->json_move(tmp);
        tmp->json_free();
    }
}