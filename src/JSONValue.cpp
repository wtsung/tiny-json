//
// Created by 吴伟聪 on 2021/5/27.
//

#include "JSONValue.h"
#include <errno.h>
//去除空格
static void json_parse_whitespace(json_context* c){
    const char* str = c->json;
    while (*str == ' ' || *str == '\r' || *str == '\n' || *str == '\t')
        str++;
    c->json = str;
}

static int json_parse_null(json_node* node, json_context* c){
    assert(*c->json == 'n');
    if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l')
        return JSON_PARSE_INVALID_VALUE;
    node->type = JSONTYPE_NULL;
    c->json += 4;
    return JSON_PARSE_OK;
}

static int json_parse_true(json_node* node, json_context* c){
    assert(*c->json == 't');
    if (c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e'){
        node->type = JSONTYPE_TRUE;
        c->json += 4;
        return JSON_PARSE_OK;
    }else{
        return JSON_PARSE_EXPECT_VALUE;
    }
}

static int json_parse_false(json_node* node, json_context* c){
    assert(*c->json == 'f');
    if (c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e'){
        node->type = JSONTYPE_FALSE;
        c->json += 5;
        return JSON_PARSE_OK;
    }else{
        return JSON_PARSE_INVALID_VALUE;
    }
}

#define ISDIGIT1TO9(ch) ((ch >= '1') && (ch <= '9'))
#define ISDIGIT(ch) ((ch >= '0') && (ch <= '9'))
static int json_parse_number(json_node* node, json_context* c){
    const char* p = c->json;
    if (*p == '-') p++; //判断负号
    if (*p == '0') {    //判断0
        p++;
    }
    else{
        if (!ISDIGIT1TO9(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }

    if (*p =='.') {
        p++;
        if (!ISDIGIT(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }

    if (*p == 'e' || *p == 'E'){
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return JSON_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p))
            p++;
    }
    errno = 0;
    node->num = strtod(c->json, NULL); //字符串转double
    //数字过大
    if (errno == ERANGE && (node->num == HUGE_VAL || node->num == -HUGE_VAL))
        return JSON_PARSE_NUMBER_TOO_BIG;
    node->type = JSONTYPE_NUMBER;
    c->json = p;
    return JSON_PARSE_OK;
}

//获取/u后的四位十六进制数
static const char* json_parse_hex4(const char* p, unsigned* u){
    *u = 0;
    for (int i = 0; i < 4; i++){
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
static void json_encode_utf8(json_context* c, unsigned u) {
    if (u <= 0x7F) //此范围码点编码对应一个字节
        c->s.push_back(u & 0xFF);
    //或是加上前缀
    else if (u <= 0x7FF) {//此范围码点编码对应2个字节
        c->s.push_back( 0xC0 | ((u >> 6) & 0xFF));//去掉后六位
        c->s.push_back( 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {//此范围码点编码对应3个字节
        c->s.push_back( 0xE0 | ((u >> 12) & 0xFF));//去掉后12位
        c->s.push_back( 0x80 | ((u >>  6) & 0x3F));//去掉后6位
        c->s.push_back( 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);//此范围码点编码对应4个字节
        c->s.push_back( 0xF0 | ((u >> 18) & 0xFF));
        c->s.push_back( 0x80 | ((u >> 12) & 0x3F));
        c->s.push_back( 0x80 | ((u >>  6) & 0x3F));
        c->s.push_back( 0x80 | ( u        & 0x3F));
    }
}

static int json_parse_string_raw(json_context* c, std::string& str){
    assert(*c->json == '\"');
    unsigned u, u2;
    c->json++;
    const char* p = c->json;
    while (1){
        char ch = *p++;
        switch (ch) {
            case '\"':
                str = c->s;
                c->s.clear();
                c->json = p;
                return JSON_PARSE_OK;
            case '\0':
                c->s.clear();
                return JSON_PARSE_MISS_DoubleDuote; //缺失结尾双引号
            case '\\':
                switch (*p++) {
                    case '\"': c->s.push_back('\"'); break;
                    case '\\': c->s.push_back('\\'); break;
                    case 'n':  c->s.push_back('\n'); break;
                    case 'b':  c->s.push_back('\b'); break;
                    case 'f':  c->s.push_back('\f'); break;
                    case 'r':  c->s.push_back('\r'); break;
                    case 't':  c->s.push_back('\t'); break;
                    case '/':  c->s.push_back('/');  break;
                    case 'u':
                        if (!(p = json_parse_hex4(p, &u)))
                            return JSON_PARSE_INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair 代理对*/ //D800-DBFF此处为高代理对
                            if (*p++ != '\\')
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (*p++ != 'u')
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (!(p = json_parse_hex4(p, &u2)))
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            if (u2 < 0xDC00 || u2 > 0xDFFF)                        //DC00-DFFF为低代理对
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        json_encode_utf8(c, u);
                        break;
                    default:
                        c->s.clear();
                        return JSON_PARSE_INVALID_STRING_ESCAPEVALUE;
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) {
                    c->s.clear();
                    return JSON_PARSE_INVALID_STRING_CHAR; //？？？
                }
                c->s.push_back(ch);
        }
    }
}

static int json_parse_string(json_node* node, json_context* c) {
    int ret;
    std::string s;
    if ((ret = json_parse_string_raw(c,s)) == JSON_PARSE_OK) {
        node->type = JSONTYPE_STRING;
        json_set_string(node, s);
    }
    return ret;
}

//free&类型置JSONTYPE_NULL
void json_free(json_node* n) {
    assert(n != nullptr);
    switch (n->type) {
        case JSONTYPE_ARRAY:
            for (int i = 0; i < n->arr.size(); i++) {
                if (n->arr[i]->type == JSONTYPE_ARRAY)
                    json_free(n->arr[i]);
                free(n->arr[i]); //释放数组中的每个指针
            }
            n->arr.clear();
            break;
        case JSONTYPE_OBJECT:
            for (auto iter = n->obj.begin(); iter != n->obj.end(); iter++) {
                json_free(iter->second);
                free(iter->second);
            }
            n->obj.clear();
            break;
        default: break;
    }
    n->type = JSONTYPE_NULL;
}

void json_init(json_node* node){
    node->type = JSONTYPE_NULL;
}

static int json_parse_value(json_node* node, json_context* c);

static int json_parse_array(json_node* node, json_context* c){
    assert(*c->json == '[');
    c->json++;
    json_parse_whitespace(c);//去除'['后的空格
    if (*c->json == ']'){
        node->type = JSONTYPE_ARRAY;
        node->arr = {};//vector为空
        c->json++;
        return JSON_PARSE_OK;
    }
    int ret;
    while(1){
        json_node* n = new json_node();
        json_init(n);
        n->arr.clear();
        if ((ret = json_parse_value(n, c)) != JSON_PARSE_OK) {
            break;
        }
        node->arr.push_back(n);
        json_parse_whitespace(c);//去除'，'前的空格
        if (*c->json == ',') {
            c->json++;
            json_parse_whitespace(c);//去除'，'后的空格
        }
        else if (*c->json == ']'){
            node->type = JSONTYPE_ARRAY;
            c->json++;
            return JSON_PARSE_OK;
        }
        else {
            for (int i = 0; i < node->arr.size(); i++)
                json_free(node->arr[i]);
            return JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
    for (int i = 0; i < node->arr.size(); i++)
        json_free(node->arr[i]);
    return ret;
}

static int json_parse_object(json_node* node, json_context* c) {
    int ret;
    assert(*c->json == '{');
    c->json++;
    std::string str;
    json_node* n;
    json_parse_whitespace(c);//去除'{'后的空格
    if (*c->json == '}'){
        c->json++;
        node->type = JSONTYPE_OBJECT;
        return JSON_PARSE_OK;
    }
    while (1){
        n = new json_node();
        if (*c->json != '"') {
            ret = JSON_PARSE_NOT_EXIST_KEY;
            break;
        }
        if ((ret = json_parse_string_raw(c, str)) != JSON_PARSE_OK)
            break;
        json_parse_whitespace(c);//去除字符串后的空格
        if (*c->json != ':'){
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        json_parse_whitespace(c);//去除':'后的空格
        if ((ret = json_parse_value(n, c)) != JSON_PARSE_OK)
            break;
        node->obj.push_back(std::pair<std::string, json_node*>(str,n));
        json_parse_whitespace(c);//去除':'后的空格
        if (*c->json == ','){
            c->json++;
            json_parse_whitespace(c);//去除','后的空格
        }
        else if (*c->json == '}'){
            c->json++;
            node->type = JSONTYPE_OBJECT;
            return JSON_PARSE_OK;
        }
        else{
            ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    for (auto iter = node->obj.begin(); iter != node->obj.end(); iter++){
        json_free(iter->second);
    }
    json_free(n);
    free(n);
    node->type = JSONTYPE_NULL;
    return ret;
}

static int json_parse_value(json_node* node, json_context* c){
    switch (*(c->json)) {
        case '\0': return JSON_PARSE_EXPECT_VALUE; //去除空格后为空
        case 'n':  return json_parse_null(node, c);
        case 't':  return json_parse_true(node, c);
        case 'f':  return json_parse_false(node, c);
        case '\"': return json_parse_string(node, c);
        case '[':  return json_parse_array(node,c);
        case '{':  return json_parse_object(node, c);
        default:   return json_parse_number(node, c);
    }
}


int json_parse(json_node* node, const char* json){
    json_context c;
    //初始化清空存储字符串
    assert(node != nullptr);
    c.json = json;
    c.s.clear();
    json_init(node);
    json_parse_whitespace(&c);//去除空格
    int ret ;
    if ((ret = json_parse_value(node, &c)) == JSON_PARSE_OK) {
        json_parse_whitespace(&c);//去除空格
        if (*(c.json) != '\0') {
            node->type = JSONTYPE_NULL;
            ret = JSON_PARSE_NOT_SINGLE_VALUE;
        }
    }
    return ret;
}


/*获得json节点类型*/
json_type json_get_type(const json_node* node){
    assert(node != nullptr);
    return node->type;
}

bool json_get_bool(const json_node* node){
    assert(node != nullptr && (node->type == JSONTYPE_TRUE || node->type == JSONTYPE_FALSE));
    return node->type == JSONTYPE_TRUE;
}
void json_set_bool(json_node* node, bool b){
    assert(node != nullptr);
    json_free(node);
    b ? node->type = JSONTYPE_TRUE : node->type = JSONTYPE_FALSE;
}


double json_get_number(const json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_NUMBER);
    return node->num;
}
void json_set_number(json_node* node, const double n){
    assert(node != nullptr);
    node->type = JSONTYPE_NUMBER;
    node->num = n;
}



void json_set_string(json_node* node, const std::string s){
    assert(node != nullptr);
    node->str = s;
    node->type = JSONTYPE_STRING;
}
std::string json_get_string(const json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_STRING);
    return node->str;
}
int json_get_string_length(const json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_STRING);
    return node->str.size();
}

int json_get_array_size(const json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    return node->arr.size();
}

int json_get_array_capacity(const json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    return node->arr.capacity();
}

//json_node* json_pushback_array_element(json_node* node){
//    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
//    node->arr.push_back()
//}

json_node* json_get_array_index(const json_node* node, int index){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    if (index < 0 || index >= node->arr.size())
        return nullptr;
    return node->arr[index];
}

void json_erase_array_element(json_node* node, int index, int count){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    auto iter_b = node->arr.begin();
    auto iter_e = node->arr.begin();
    if (count <= 0)
        return;
    for (int i = 0; i < index; i++)
        iter_b++;
    iter_e = iter_b;
    for (int i = 0; i < count; i++)
        iter_e++;
    node->arr.erase(iter_b,iter_e);
}

void json_pushback_array_element(json_node* node, json_node* v){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    node->arr.push_back(v);
}

void json_popback_array_element(json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    json_free(node->arr[node->arr.size() - 1]);
    node->arr.pop_back();
}

void json_insert_array_element(json_node* node, json_node* v, int index){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    node->arr.insert(node->arr.begin() + index, v);
}

void json_clear_array(json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_ARRAY);
    for (int i = 0; i < node->arr.size(); i++){
        free(node->arr[i]);
    }
    node->arr.clear();
}

int json_get_object_size(const json_node* n){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT);
    return n->obj.size();
}

int json_get_object_capacity(const json_node* n){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT);
    return n->obj.capacity();
}

const std::string json_get_object_key(const json_node* n, int index){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT && index >= 0 && index < n->obj.size());
    int i = 0;
    auto iter = n->obj.begin();
    while (i++ < index){
        iter++;
    }
    return iter->first;
}
int json_get_object_key_length(const json_node* n, int index){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT && index >= 0 && index < n->obj.size());
    int i = 0;
    auto iter = n->obj.begin();
    while (i++ < index){
        iter++;
    }
    return iter->first.size();
}
json_node* json_get_object_value(const json_node* n, int index){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT && index >= 0 && index < n->obj.size());
    int i = 0;
    auto iter = n->obj.begin();
    while (i++ < index){
        iter++;
    }
    return iter->second;
}

void json_set_object_value(json_node* n, std::string key, json_node* v){
    assert(n != nullptr && n->type == JSONTYPE_OBJECT);
    auto iter = n->obj.begin();
    while (iter != n->obj.end()){
        if (iter->first == key) {
            json_free(iter->second);
            iter->second = v;
            return;
        }
        iter++;
    }
    n->obj.push_back(std::pair<std::string, json_node*>(key, v));
}

int json_find_object_index(const json_node* node, const std::string str) {
    int index = 0;
    assert(node != nullptr && node->type == JSONTYPE_OBJECT);
    auto iter = node->obj.begin();
    for (; iter != node->obj.end(); iter++) {
        if (iter->first == str)
            return index;
        index++;
    }
    return JSON_PARSE_NOT_EXIST_KEY;
}
json_node* json_find_object_value(json_node* node, const std::string str) {
    assert(node != nullptr && node->type == JSONTYPE_OBJECT);
    auto iter = node->obj.begin();
    for (; iter != node->obj.end(); iter++) {
        if (iter->first == str)
            return iter->second;
    }
    return nullptr;
}

void json_clear_object(json_node* node){
    assert(node != nullptr && node->type == JSONTYPE_OBJECT);
    json_free(node);
    node->obj.clear();
    node->type = JSONTYPE_OBJECT; //不改变node的类型
}

void json_remove_object_value(json_node* node, int index){
    assert(node != nullptr && node->type == JSONTYPE_OBJECT);
    auto iter = node->obj.begin();
    for (int i = 0; i < index; i++){
        iter++;
    }
    json_free(iter->second);
    node->obj.erase(iter);
}

static void json_stringify_string(const json_node* node, std::string& str) {
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    assert(node != nullptr && node->type == JSONTYPE_STRING);
    str.push_back('"');
    for(int i = 0; i < node->str.size(); i++){
        char ch = node->str[i];
        switch (ch) {
            case '\"': str += '\\'; str += '\"'; break;
            case '\\': str += '\\'; str += '\\'; break;
            case '\b': str += '\\'; str += 'b'; break;
            case '\f': str += '\\'; str += 'f'; break;
            case '\n': str += '\\'; str += 'n'; break;
            case '\r': str += '\\'; str += 'r'; break;
            case '\t': str += '\\'; str += 't'; break;
            default:
                if (ch < 0x20) {
                    str += '\\'; str += 'u'; str += '0'; str += '0';
                    str += hex_digits[ch >> 4];
                    str += hex_digits[ch & 15];
                }
                else
                    str += node->str[i];
        }
    }
    str.push_back('"');
}
static void json_stringify_number(const json_node* node, std::string& str){
    char tmp[32];
    sprintf(tmp, "%.17g", node->num);
    str += tmp;
//    int tmp_len = strlen(tmp);
//    for (int i = 0; i < tmp_len; i++)
//        str.push_back(tmp[i]);
}
static void json_stringify_value(const json_node* node, std::string& str){
    switch (node->type) {
        case JSONTYPE_NULL: str += "null"; break;
        case JSONTYPE_TRUE: str += "true"; break;
        case JSONTYPE_FALSE: str += "false"; break;
        case JSONTYPE_NUMBER: json_stringify_number(node, str); break;
        case JSONTYPE_STRING: json_stringify_string(node, str); break;
        case JSONTYPE_ARRAY:
            str += '[';
            for (int i = 0; i < node->arr.size(); i++){
                if (i > 0)
                    str += ',';
                json_stringify_value(node->arr[i], str);
            }
            str += ']';
            break;
        case JSONTYPE_OBJECT:
            str += '{';
            for (int i = 0; i < node->obj.size(); i++){
                if (i > 0)
                    str += ',';
                str += '"'; str += node->obj[i].first; str += '"';
                str += ':';
                json_stringify_value(node->obj[i].second, str);
            }
            str += '}';
            break;
    }
}
std::string json_stringify(const json_node* node){
    assert(node != nullptr);
    std::string str;
    json_stringify_value(node, str);
    return str;
}

int json_is_equal(const json_node* lhs, const json_node* rhs) {
    assert(lhs != nullptr && rhs != nullptr);
    if (lhs->type != rhs->type)
        return 0;
    auto iter_lhs = lhs->obj.begin();
    auto iter_rhs = rhs->obj.begin();
    switch (lhs->type) {
        case JSONTYPE_STRING:
            return lhs->str == rhs->str;
        case JSONTYPE_NUMBER:
            return lhs->num == rhs->num;
        case JSONTYPE_ARRAY:
            if (lhs->arr.size() != rhs->arr.size())
                return 0;
            for (int i = 0; i < lhs->arr.size(); i++)
                if (!json_is_equal(lhs->arr[i], rhs->arr[i]))
                    return 0;
            return 1;
        case JSONTYPE_OBJECT:
            if (lhs->obj.size() != rhs->obj.size())
                return 0;
            int index;
            for (; iter_lhs != lhs->obj.end(); iter_lhs++) {
                index = json_find_object_index(rhs, iter_lhs->first);
                if (!json_is_equal(iter_lhs->second, rhs->obj[index].second))
                    return 0;
//                if ((iter_lhs->first != iter_rhs->first) || !json_is_equal(iter_lhs->second, iter_rhs->second))
//                    return 0;
            }
            return 1;
        default:
            return 1;
    }
}

void json_copy(json_node* dst, const json_node* src) {
    int i;
    assert(src != nullptr && dst != nullptr && src != dst);
    auto iter = src->obj.begin();
    switch (src->type) {
        case JSONTYPE_STRING:
            dst->type = JSONTYPE_STRING;
            json_set_string(dst, src->str);
            break;
        case JSONTYPE_ARRAY:
            dst->type = JSONTYPE_ARRAY;
            json_node* tmp_arr;
            for (i = 0; i < src->arr.size(); i++){
                tmp_arr = new json_node();
                memcpy(tmp_arr, src->arr[i], sizeof(json_node));
                dst->arr.push_back(tmp_arr);
            }
            break;
        case JSONTYPE_OBJECT:
            dst->type = JSONTYPE_OBJECT;
            json_node* tmp_obj;
            for (; iter != src->obj.end(); iter++){
                tmp_obj = new json_node();
                json_copy(tmp_obj,iter->second);
//                memcpy(tmp_obj, iter->second, sizeof(json_node));
                dst->obj.push_back(std::pair<std::string, json_node*>(iter->first, tmp_obj));
            }
            break;
        default:
            json_free(dst);
            memcpy(dst, src, sizeof(json_node));
            break;
    }
}

void json_move(json_node* dst, json_node* src){
    assert(dst != nullptr && src != nullptr && src != dst);
    json_free(dst);
    json_copy(dst, src);
    json_init(src); //源释放
}

void json_swap(json_node* lhs, json_node* rhs){
    assert(lhs != nullptr && rhs != nullptr);
    if (lhs != rhs){
        json_node* tmp = new json_node();
        json_move(tmp, lhs);
        json_move(lhs, rhs);
        json_move(rhs, tmp);
        free(tmp);
    }
}