//
// Created by 吴伟聪 on 2021/5/27.
//

#include "JsonValue.h"
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
    node->json_set_null();
    c->json += 4;
    return JSON_PARSE_OK;
}

static int json_parse_true(json_node* node, json_context* c){
    assert(*c->json == 't');
    if (c->json[1] == 'r' && c->json[2] == 'u' && c->json[3] == 'e'){
        node->json_set_bool(1);
        c->json += 4;
        return JSON_PARSE_OK;
    }else{
        return JSON_PARSE_EXPECT_VALUE;
    }
}

static int json_parse_false(json_node* node, json_context* c){
    assert(*c->json == 'f');
    if (c->json[1] == 'a' && c->json[2] == 'l' && c->json[3] == 's' && c->json[4] == 'e'){
        node->json_set_bool(0);
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
    double num_str = strtod(c->json, NULL); //字符串转double
    //数字过大
    if (errno == ERANGE && (num_str == HUGE_VAL || num_str == -HUGE_VAL))
        return JSON_PARSE_NUMBER_TOO_BIG;
    node->json_set_number(num_str);
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
static void json_encode_utf8(std::string& str, unsigned u) {
    if (u <= 0x7F) //此范围码点编码对应一个字节
        str.push_back(u & 0xFF);
        //或是加上前缀
    else if (u <= 0x7FF) {//此范围码点编码对应2个字节
        str.push_back( 0xC0 | ((u >> 6) & 0xFF));//去掉后六位
        str.push_back( 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {//此范围码点编码对应3个字节
        str.push_back( 0xE0 | ((u >> 12) & 0xFF));//去掉后12位
        str.push_back( 0x80 | ((u >>  6) & 0x3F));//去掉后6位
        str.push_back( 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);//此范围码点编码对应4个字节
        str.push_back( 0xF0 | ((u >> 18) & 0xFF));
        str.push_back( 0x80 | ((u >> 12) & 0x3F));
        str.push_back( 0x80 | ((u >>  6) & 0x3F));
        str.push_back( 0x80 | ( u        & 0x3F));
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
                c->json = p;
                return JSON_PARSE_OK;
            case '\0':
                str.clear();
                return JSON_PARSE_MISS_DoubleDuote; //缺失结尾双引号
            case '\\':
                switch (*p++) {
                    case '\"': str.push_back('\"'); break;
                    case '\\': str.push_back('\\'); break;
                    case 'n':  str.push_back('\n'); break;
                    case 'b':  str.push_back('\b'); break;
                    case 'f':  str.push_back('\f'); break;
                    case 'r':  str.push_back('\r'); break;
                    case 't':  str.push_back('\t'); break;
                    case '/':  str.push_back('/');  break;
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
                        json_encode_utf8(str, u);
                        break;
                    default:
                        str.clear();
                        return JSON_PARSE_INVALID_STRING_ESCAPEVALUE;
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) {
                    str.clear();
                    return JSON_PARSE_INVALID_STRING_CHAR; //？？？
                }
                str.push_back(ch);
        }
    }
}

static int json_parse_string(json_node* node, json_context* c) {
    int ret;
    std::string s;
    if ((ret = json_parse_string_raw(c,s)) == JSON_PARSE_OK) {
        node->json_set_string(s);
    }
    return ret;
}

//free&类型置JSONTYPE_NULL
void json_node::json_free() {
    assert(this != nullptr);
    switch (this->type) {
        case JSONTYPE_ARRAY:
            for (int i = 0; i < this->arr.size(); i++) {
                if (this->arr[i]->type == JSONTYPE_ARRAY)
                    this->arr[i]->json_free();
                free(this->arr[i]); //释放数组中的每个指针
            }
            this->arr.clear();
            break;
        case JSONTYPE_OBJECT:
            for (auto iter = this->obj.begin(); iter != this->obj.end(); iter++) {
                iter->second->json_free();
                free(iter->second);
            }
            this->obj.clear();
            break;
        default: break;
    }
    this->type = JSONTYPE_NULL;
}

void json_node::json_init(){
    this->type = JSONTYPE_NULL;
}

static int json_parse_value(json_node* node, json_context* c);

static int json_parse_array(json_node* node, json_context* c){
    assert(*c->json == '[');
    node->json_set_array();
    c->json++;
    json_parse_whitespace(c);//去除'['后的空格
    if (*c->json == ']'){
        node->json_set_array({});
        c->json++;
        return JSON_PARSE_OK;
    }
    int ret;
    while(1){
        json_node* n = new json_node();
        n->json_init();
        if ((ret = json_parse_value(n, c)) != JSON_PARSE_OK) {
            break;
        }
        node->json_pushback_array_element(n);
        json_parse_whitespace(c);//去除'，'前的空格
        if (*c->json == ',') {
            c->json++;
            json_parse_whitespace(c);//去除'，'后的空格
        }
        else if (*c->json == ']'){
            node->json_set_array();
            c->json++;
            return JSON_PARSE_OK;
        }
        else {
            for (int i = 0; i < node->json_get_array_size(); i++)
                node->json_get_array_index(i)->json_free();
            node->json_free();
            return JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
    for (int i = 0; i < node->json_get_array_size(); i++)
        node->json_get_array_index(i)->json_free();
    node->json_free();
    return ret;
}

static int json_parse_object(json_node* node, json_context* c) {
    int ret;
    assert(*c->json == '{');
    node->json_set_object();
    c->json++;
    std::string str;
    json_node* n;
    json_parse_whitespace(c);//去除'{'后的空格
    if (*c->json == '}'){
        c->json++;
        node->json_set_object({});
        return JSON_PARSE_OK;
    }
    while (1){
        n = new json_node();
        str.clear();
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
        node->json_pushback_object_element(str,n);
        json_parse_whitespace(c);//去除':'后的空格
        if (*c->json == ','){
            c->json++;
            json_parse_whitespace(c);//去除','后的空格
        }
        else if (*c->json == '}'){
            c->json++;
            node->json_set_object();
            return JSON_PARSE_OK;
        }
        else{
            ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    for (int i = 0; i < node->json_get_object_size(); i++){
        node->json_get_object_value(i)->json_free();
    }
    node->json_free();
    n->json_free();
    return ret;
}

static int json_parse_value(json_node* node, json_context* c){
    switch (*(c->json)) {
        case '\0': return JSON_PARSE_EXPECT_VALUE; //去除空格后为空
        case 'n':  return json_parse_null(node, c);
        case 't':  return json_parse_true(node, c);
        case 'f':  return json_parse_false(node, c);
        case '\"': return json_parse_string(node, c);
        case '[':  return json_parse_array(node, c);
        case '{':  return json_parse_object(node, c);
        default:   return json_parse_number(node, c);
    }
}


int json_node::json_parse(const char* json){
    json_context c;
    //初始化清空存储字符串
    assert(this != nullptr);
    c.json = json;
    this->json_init();
    json_parse_whitespace(&c);//去除空格
    int ret ;
    if ((ret = json_parse_value(this, &c)) == JSON_PARSE_OK) {
        json_parse_whitespace(&c);//去除空格
        if (*(c.json) != '\0') {
            this->json_set_null();
            ret = JSON_PARSE_NOT_SINGLE_VALUE;
        }
    }
    return ret;
}


/*获得json节点类型*/
json_type json_node::json_get_type() const
{
    assert(this != nullptr);
    return this->type;
}

void json_node::json_set_null(){
    assert(this != nullptr);
    this->json_free();
    this->type = JSONTYPE_NULL;
}

bool json_node::json_get_bool() const{
    assert(this != nullptr && (this->type == JSONTYPE_TRUE || this->type == JSONTYPE_FALSE));
    return this->type == JSONTYPE_TRUE;
}

void json_node::json_set_bool(bool b){
    assert(this != nullptr);
    this->json_free();
    b ? this->type = JSONTYPE_TRUE : this->type = JSONTYPE_FALSE;
}


double json_node::json_get_number() const {
    assert(this != nullptr && this->type == JSONTYPE_NUMBER);
    return this->num;
}
void json_node::json_set_number(double n){
    assert(this != nullptr);
    this->type = JSONTYPE_NUMBER;
    this->num = n;
}

void json_node::json_set_string(std::string s){
    assert(this != nullptr);
    this->str = s;
    this->type = JSONTYPE_STRING;
}
std::string json_node::json_get_string() const
{
    assert(this != nullptr && this->type == JSONTYPE_STRING);
    return this->str;
}
int json_node::json_get_string_length() const
{
    assert(this != nullptr && this->type == JSONTYPE_STRING);
    return this->str.size();
}

void json_node::json_set_array(){
    assert(this != nullptr);
    this->type = JSONTYPE_ARRAY;
}

void json_node::json_set_array(std::vector<json_node*> arr){
    assert(this != nullptr);
    this->arr = arr;
    this->type = JSONTYPE_ARRAY;
}

int json_node::json_get_array_size() const
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    return this->arr.size();
}

int json_node::json_get_array_capacity() const
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    return this->arr.capacity();
}

json_node* json_node::json_get_array_index(int index) const
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    if (index < 0 || index >= this->arr.size())
        return nullptr;
    return this->arr[index];
}

void json_node::json_erase_array_element(int index, int count){
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    auto iter_b = this->arr.begin();
    int i = 0;
    int j = 0;
    if (count <= 0)
        return;
    while (i++ < index)
        iter_b++;
    auto iter_e = iter_b;
    while (j++ < count)
        iter_e++;
    this->arr.erase(iter_b,iter_e);
}

void json_node::json_pushback_array_element(json_node* v)
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    this->arr.push_back(v);
}

void json_node::json_popback_array_element()
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    this->arr[this->arr.size() - 1]->json_free();
    this->arr.pop_back();
}

void json_node::json_insert_array_element(json_node* v, int index)
{
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    this->arr.insert(this->arr.begin() + index, v);
}

void json_node::json_clear_array(){
    assert(this != nullptr && this->type == JSONTYPE_ARRAY);
    for (int i = 0; i < this->arr.size(); i++)
        free(this->arr[i]);//????
    this->arr.clear();
}

void json_node::json_set_object(){
    assert(this != nullptr);
    this->type = JSONTYPE_OBJECT;
}
void json_node::json_set_object(std::vector<std::pair<std::string, json_node*>> obj){
    assert(this != nullptr);
    this->obj = obj;
    this->type = JSONTYPE_OBJECT;
}

int json_node::json_get_object_size() const
{
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    return this->obj.size();
}

int json_node::json_get_object_capacity() const
{
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    return this->obj.capacity();
}

std::string json_node::json_get_object_key(int index) const
{
    assert(this != nullptr && this->type == JSONTYPE_OBJECT && index >= 0 && index < this->obj.size());
    int i = 0;
    auto iter = this->obj.begin();
    while (i++ < index)
        iter++;
    return iter->first;
}
int json_node::json_get_object_key_length(int index) const
{
    assert(this != nullptr && this->type == JSONTYPE_OBJECT && index >= 0 && index < this->obj.size());
    int i = 0;
    auto iter = this->obj.begin();
    while (i++ < index)
        iter++;
    return iter->first.size();
}

json_node* json_node::json_get_object_value(int index) const
{
    assert(this != nullptr && this->type == JSONTYPE_OBJECT && index >= 0 && index < this->obj.size());
    int i = 0;
    auto iter = this->obj.begin();
    while (i++ < index)
        iter++;
    return iter->second;
}

void json_node::json_set_object_value(std::string key, json_node* v){
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    auto iter = this->obj.begin();
    while (iter != this->obj.end()){
        if (iter->first == key) {
            iter->second->json_free();
            iter->second = v;
            return;
        }
        iter++;
    }
    this->obj.push_back(std::pair<std::string, json_node*>(key, v));
}

int json_node::json_find_object_index(const std::string str) const
{
    int index = 0;
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    for (auto iter : this->obj){
        if (iter.first == str)
            return index;
        index++;
    }
    return JSON_PARSE_NOT_EXIST_KEY;
}

json_node* json_node::json_find_object_value(const std::string str) {
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    for (auto iter : this->obj){
        if (iter.first == str)
            return iter.second;
    }
    return nullptr;
}

void json_node::json_clear_object(){
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    this->json_free();
    this->obj.clear();
    this->type = JSONTYPE_OBJECT; //不改变node的类型
}

void json_node::json_remove_object_value(int index){
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    auto iter = this->obj.begin();
    int i = 0;
    while (i++ < index)
        iter++;
    iter->second->json_free();
    this->obj.erase(iter);
}

void json_node::json_pushback_object_element(std::string key, json_node* v){
    assert(this != nullptr && this->type == JSONTYPE_OBJECT);
    this->obj.push_back(std::pair<std::string, json_node*>(key, v));
}

static void json_stringify_string(const json_node* node, std::string& str) {
    static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    assert(node != nullptr && node->json_get_type() == JSONTYPE_STRING);
    str.push_back('"');
    for(int i = 0; i < node->json_get_string_length(); i++){
        char ch = node->json_get_string()[i];
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
                    str += node->json_get_string()[i];
        }
    }
    str.push_back('"');
}
static void json_stringify_number(const json_node* node, std::string& str){
    char tmp[32];
    sprintf(tmp, "%.17g", node->json_get_number());
    str += tmp;
}

static void json_stringify_value(const json_node* node, std::string& str){
    switch (node->json_get_type()) {
        case JSONTYPE_NULL: str += "null"; break;
        case JSONTYPE_TRUE: str += "true"; break;
        case JSONTYPE_FALSE: str += "false"; break;
        case JSONTYPE_NUMBER: json_stringify_number(node, str); break;
        case JSONTYPE_STRING: json_stringify_string(node, str); break;
        case JSONTYPE_ARRAY:
            str += '[';
            for (int i = 0; i < node->json_get_array_size(); i++){
                if (i > 0)
                    str += ',';
                json_stringify_value(node->json_get_array_index(i), str);
            }
            str += ']';
            break;
        case JSONTYPE_OBJECT:
            str += '{';
            for (int i = 0; i < node->json_get_object_size(); i++){
                if (i > 0)
                    str += ',';
                str += '"'; str += node->json_get_object_key(i); str += '"';
                str += ':';
                json_stringify_value(node->json_get_object_value(i), str);
            }
            str += '}';
            break;
    }
}
std::string json_node::json_stringify() const
{
    assert(this != nullptr);
    std::string str;
    json_stringify_value(this, str);
    return str;
}

int json_node::json_is_equal(json_node* rhs) const
{
    assert(this != nullptr && rhs != nullptr);
    if (this->type != rhs->type)
        return 0;
    auto iter = this->obj.begin();
    auto iter_rhs = rhs->obj.begin();
    switch (this->type) {
        case JSONTYPE_STRING:
            return this->str == rhs->str;
        case JSONTYPE_NUMBER:
            return this->num == rhs->num;
        case JSONTYPE_ARRAY:
            if (this->arr.size() != rhs->arr.size())
                return 0;
            for (int i = 0; i < this->arr.size(); i++)
                if (!this->arr[i]->json_is_equal(rhs->arr[i]))
                    return 0;
            return 1;
        case JSONTYPE_OBJECT:
            if (this->obj.size() != rhs->obj.size())
                return 0;
            int index;
            for (; iter != this->obj.end(); iter++) {
                index = rhs->json_find_object_index(iter->first);
                if (!iter->second->json_is_equal(rhs->obj[index].second))
                    return 0;
            }
            return 1;
        default:
            return 1;
    }
}

void json_node::json_copy(json_node* src)
{
    int i;
    assert(src != nullptr && this != nullptr && src != this);
    auto iter = src->obj.begin();
    switch (src->type) {
        case JSONTYPE_STRING:
            this->json_set_string(src->str);
            break;
        case JSONTYPE_ARRAY:
            this->type = JSONTYPE_ARRAY;
            json_node* tmp_arr;
            for (i = 0; i < src->arr.size(); i++){
                tmp_arr = new json_node();
                memcpy(tmp_arr, src->arr[i], sizeof(json_node));
                this->json_pushback_array_element(tmp_arr);
            }
            break;
        case JSONTYPE_OBJECT:
            this->type = JSONTYPE_OBJECT;
            json_node* tmp_obj;
            for (; iter != src->obj.end(); iter++){
                tmp_obj = new json_node();
                tmp_obj->json_copy(iter->second);
                this->json_pushback_object_element(iter->first, tmp_obj);
            }
            break;
        default:
            this->json_free();
            memcpy(this, src, sizeof(json_node));
            break;
    }
}

void json_node::json_move(json_node* src){
    assert(this != nullptr && src != nullptr && src != this);
    this->json_free();
    this->json_copy(src);
    src->json_init(); //源释放
}

void json_node::json_swap(json_node* rhs){
    assert(this != nullptr && rhs != nullptr);
    if (this != rhs){
        json_node* tmp = new json_node();
        tmp->json_move(this);
        this->json_move(rhs);
        rhs->json_move(tmp);
        tmp->json_free();
    }
}