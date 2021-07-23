#pragma once
#include "tiny_json.h"


class Parser final {
public:
    Parser() = default;
    Parser(const JsonContext& c);
    Parser(const Parser& parse) = delete;
    Parser& operator=(const Parser& parse) = delete;
    ~Parser() = default;
    int parse(JsonNode& node);

private:
    void parse_whitespace();
    int parse_null(JsonNode* node);
    int parse_true(JsonNode* node);
    int parse_false(JsonNode* node);
    int parse_number(JsonNode* node);
    const char* parse_hex4(const char* p, unsigned* u);
    void encode_utf8(std::string& str, unsigned u);
    int parse_string_raw(std::string& str);
    int parse_string(JsonNode* node);
    int parse_array(JsonNode* node);
    int parse_object(JsonNode* node);
    int parse_value(JsonNode* node);
    JsonContext ctx;
};
