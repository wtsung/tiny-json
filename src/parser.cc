#include "parser.h"
#include <cerrno>

Parser::Parser(const JsonContext& c) {
    ctx.json = c.json;
}

void Parser::parse_whitespace() {
    const char* str = ctx.json;
    while (*str == ' ' || *str == '\r' || *str == '\n' || *str == '\t') {
        str++;
    }
    ctx.json = str;
}

int Parser::parse_null(JsonNode* node) {
    assert(*ctx.json == 'n');
    if (ctx.json[1] != 'u' || ctx.json[2] != 'l' || ctx.json[3] != 'l') {
        return JSON_PARSE_INVALID_VALUE;
    }
    node->set_null();
    ctx.json += 4;
    return JSON_PARSE_OK;
}

int Parser::parse_true(JsonNode* node) {
    assert(*ctx.json == 't');
    if (ctx.json[1] == 'r' && ctx.json[2] == 'u' && ctx.json[3] == 'e') {
        node->set_bool(true);
        ctx.json += 4;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_EXPECT_VALUE;
    }
}

int Parser::parse_false(JsonNode* node) {
    assert(*ctx.json == 'f');
    if (ctx.json[1] == 'a' && ctx.json[2] == 'l' && ctx.json[3] == 's' && ctx.json[4] == 'e') {
        node->set_bool(false);
        ctx.json += 5;
        return JSON_PARSE_OK;
    } else {
        return JSON_PARSE_INVALID_VALUE;
    }
}

#define ISDIGIT1TO9(ch) (((ch) >= '1') && ((ch) <= '9'))
#define ISDIGIT(ch) (((ch) >= '0') && ((ch) <= '9'))
int Parser::parse_number(JsonNode* node) {
    const char* p = ctx.json;
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
    double num_str = strtod(ctx.json, nullptr);//str to double
    if (errno == ERANGE && (num_str == HUGE_VAL || num_str == -HUGE_VAL)) {
        return JSON_PARSE_NUMBER_TOO_BIG;
    }
    node->set_number(num_str);
    ctx.json = p;
    return JSON_PARSE_OK;
}

const char* Parser::parse_hex4(const char* p, unsigned* u) {
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

void Parser::encode_utf8(std::string& str, unsigned u) {
    if (u <= 0x7F) {
        str.push_back(u & 0xFF);
    }
    //或是加上前缀
    else if (u <= 0x7FF) {
        str.push_back(0xC0 | ((u >> 6) & 0xFF));
        str.push_back(0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        str.push_back(0xE0 | ((u >> 12) & 0xFF));
        str.push_back(0x80 | ((u >> 6) & 0x3F));
        str.push_back(0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);//此范围码点编码对应4个字节
        str.push_back(0xF0 | ((u >> 18) & 0xFF));
        str.push_back(0x80 | ((u >> 12) & 0x3F));
        str.push_back(0x80 | ((u >> 6) & 0x3F));
        str.push_back(0x80 | (u & 0x3F));
    }
}

int Parser::parse_string_raw(std::string& str) {
    assert(*ctx.json == '\"');
    unsigned u, u2;
    ctx.json++;
    const char* p = ctx.json;
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                ctx.json = p;
                return JSON_PARSE_OK;
            case '\0':
                str.clear();
                return JSON_PARSE_MISS_DOUBLEDUOTE;
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
                        /* surrogate pair */
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\') {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (*p++ != 'u') {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (!(p = parse_hex4(p, &u2))) {
                                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if (u2 < 0xDC00 || u2 > 0xDFFF) {
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

int Parser::parse_string(JsonNode* node) {
    int ret;
    std::string s;
    if ((ret = parse_string_raw(s)) == JSON_PARSE_OK) {
        node->set_string(s);
    }
    return ret;
}

int Parser::parse_array(JsonNode* node) {
    assert(*ctx.json == '[');
    node->set_array();
    ctx.json++;
    parse_whitespace();
    if (*ctx.json == ']') {
        node->set_array({});
        ctx.json++;
        return JSON_PARSE_OK;
    }
    int ret;
    while (true) {
        auto n = new JsonNode();
        n->json_init();
        if ((ret = parse_value(n)) != JSON_PARSE_OK) {
            break;
        }
        node->pushback_array_element(n);
        parse_whitespace();
        if (*ctx.json == ',') {
            ctx.json++;
            parse_whitespace();
        } else if (*ctx.json == ']') {
            node->set_array();
            ctx.json++;
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

int Parser::parse_object(JsonNode* node) {
    int ret;
    assert(*ctx.json == '{');
    node->set_object();
    ctx.json++;
    std::string str;
    JsonNode* n;
    parse_whitespace();
    if (*ctx.json == '}') {
        ctx.json++;
        node->set_object();
        return JSON_PARSE_OK;
    }
    while (true) {
        n = new JsonNode();
        str.clear();
        if (*ctx.json != '"') {
            ret = JSON_PARSE_NOT_EXIST_KEY;
            break;
        }
        if ((ret = parse_string_raw(str)) != JSON_PARSE_OK) {
            break;
        }
        parse_whitespace();
        if (*ctx.json != ':') {
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        ctx.json++;
        parse_whitespace();
        if ((ret = parse_value(n)) != JSON_PARSE_OK) {
            break;
        }
        node->pushback_object_element(str, n);
        parse_whitespace();
        if (*ctx.json == ',') {
            ctx.json++;
            parse_whitespace();
        } else if (*ctx.json == '}') {
            ctx.json++;
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

int Parser::parse_value(JsonNode* node) {
    switch (*(ctx.json)) {
        case '\0':
            return JSON_PARSE_EXPECT_VALUE;
        case 'n':
            return parse_null(node);
        case 't':
            return parse_true(node);
        case 'f':
            return parse_false(node);
        case '\"':
            return parse_string(node);
        case '[':
            return parse_array(node);
        case '{':
            return parse_object(node);
        default:
            return parse_number(node);
    }
}

int Parser::parse(JsonNode& node) {
    parse_whitespace();
    int ret;
    if ((ret = parse_value(&node)) == JSON_PARSE_OK) {
        parse_whitespace();
        if (*(ctx.json) != '\0') {
            node.set_null();
            ret = JSON_PARSE_NOT_SINGLE_VALUE;
        }
    }
    return ret;
}
