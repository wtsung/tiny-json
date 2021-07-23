#include "tiny_json.h"
#include "parser.h"

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

int JsonNode::json_parse(const char* json) {
    JsonContext ctx{};
    ctx.json = json;
    this->json_init();
    Parser p(ctx);
    int ret;
    ret = p.parse(*this);
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
        this->object.emplace_back(std::pair<std::string, JsonNode*>(node.first, node_tmp));
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