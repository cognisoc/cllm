/*
 * json.c - Minimal JSON parser/writer implementation
 */

#include "json.h"
#include "string.h"

// Helper: check if character is whitespace
static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Helper: check if character is digit
static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Helper: skip whitespace
static void skip_whitespace(json_parser_t* p) {
    while (p->pos < p->len && is_whitespace(p->input[p->pos])) {
        p->pos++;
    }
}

// Initialize parser
void json_parser_init(json_parser_t* p, const char* input, size_t len) {
    p->input = input;
    p->len = len;
    p->pos = 0;
    p->current = JSON_TOK_EOF;
    p->string_buf[0] = '\0';
    p->string_len = 0;
    p->number_value = 0.0;
    p->error[0] = '\0';
}

// Parse string literal
static json_token_t parse_string(json_parser_t* p) {
    p->pos++; // Skip opening quote
    p->string_len = 0;

    while (p->pos < p->len) {
        char c = p->input[p->pos];

        if (c == '"') {
            p->pos++; // Skip closing quote
            p->string_buf[p->string_len] = '\0';
            return JSON_TOK_STRING;
        }

        if (c == '\\' && p->pos + 1 < p->len) {
            p->pos++;
            c = p->input[p->pos];
            switch (c) {
                case '"':  c = '"'; break;
                case '\\': c = '\\'; break;
                case '/':  c = '/'; break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                case 't':  c = '\t'; break;
                case 'u':
                    // Skip unicode escape (just consume 4 hex digits)
                    p->pos += 4;
                    c = '?'; // Replace with placeholder
                    break;
                default:
                    break;
            }
        }

        if (p->string_len < sizeof(p->string_buf) - 1) {
            p->string_buf[p->string_len++] = c;
        }
        p->pos++;
    }

    strncpy(p->error, "Unterminated string", sizeof(p->error));
    return JSON_TOK_ERROR;
}

// Parse number
static json_token_t parse_number(json_parser_t* p) {
    size_t start = p->pos;
    bool negative = false;
    bool has_decimal = false;
    bool has_exponent = false;

    if (p->input[p->pos] == '-') {
        negative = true;
        p->pos++;
    }

    // Integer part
    while (p->pos < p->len && is_digit(p->input[p->pos])) {
        p->pos++;
    }

    // Decimal part
    if (p->pos < p->len && p->input[p->pos] == '.') {
        has_decimal = true;
        p->pos++;
        while (p->pos < p->len && is_digit(p->input[p->pos])) {
            p->pos++;
        }
    }

    // Exponent part
    if (p->pos < p->len && (p->input[p->pos] == 'e' || p->input[p->pos] == 'E')) {
        has_exponent = true;
        p->pos++;
        if (p->pos < p->len && (p->input[p->pos] == '+' || p->input[p->pos] == '-')) {
            p->pos++;
        }
        while (p->pos < p->len && is_digit(p->input[p->pos])) {
            p->pos++;
        }
    }

    // Parse the number manually (no strtod in unikernel)
    p->number_value = 0.0;
    double fraction = 0.0;
    double divisor = 10.0;
    int exponent = 0;
    bool exp_negative = false;
    bool in_decimal = false;
    bool in_exponent = false;

    for (size_t i = start; i < p->pos; i++) {
        char c = p->input[i];

        if (c == '-' && i == start) {
            continue;
        } else if (c == '.') {
            in_decimal = true;
        } else if (c == 'e' || c == 'E') {
            in_exponent = true;
        } else if (c == '+') {
            continue;
        } else if (c == '-' && in_exponent) {
            exp_negative = true;
        } else if (is_digit(c)) {
            if (in_exponent) {
                exponent = exponent * 10 + (c - '0');
            } else if (in_decimal) {
                fraction += (c - '0') / divisor;
                divisor *= 10.0;
            } else {
                p->number_value = p->number_value * 10.0 + (c - '0');
            }
        }
    }

    p->number_value += fraction;

    // Apply exponent
    if (has_exponent) {
        if (exp_negative) exponent = -exponent;
        double mult = 1.0;
        for (int i = 0; i < (exponent > 0 ? exponent : -exponent); i++) {
            mult *= 10.0;
        }
        if (exponent > 0) {
            p->number_value *= mult;
        } else {
            p->number_value /= mult;
        }
    }

    if (negative) {
        p->number_value = -p->number_value;
    }

    return JSON_TOK_NUMBER;
}

// Get next token
json_token_t json_next_token(json_parser_t* p) {
    skip_whitespace(p);

    if (p->pos >= p->len) {
        p->current = JSON_TOK_EOF;
        return p->current;
    }

    char c = p->input[p->pos];

    switch (c) {
        case '{':
            p->pos++;
            p->current = JSON_TOK_LBRACE;
            break;
        case '}':
            p->pos++;
            p->current = JSON_TOK_RBRACE;
            break;
        case '[':
            p->pos++;
            p->current = JSON_TOK_LBRACKET;
            break;
        case ']':
            p->pos++;
            p->current = JSON_TOK_RBRACKET;
            break;
        case ':':
            p->pos++;
            p->current = JSON_TOK_COLON;
            break;
        case ',':
            p->pos++;
            p->current = JSON_TOK_COMMA;
            break;
        case '"':
            p->current = parse_string(p);
            break;
        case 't':
            if (p->pos + 4 <= p->len &&
                p->input[p->pos + 1] == 'r' &&
                p->input[p->pos + 2] == 'u' &&
                p->input[p->pos + 3] == 'e') {
                p->pos += 4;
                p->current = JSON_TOK_TRUE;
            } else {
                p->current = JSON_TOK_ERROR;
                strncpy(p->error, "Invalid token", sizeof(p->error));
            }
            break;
        case 'f':
            if (p->pos + 5 <= p->len &&
                p->input[p->pos + 1] == 'a' &&
                p->input[p->pos + 2] == 'l' &&
                p->input[p->pos + 3] == 's' &&
                p->input[p->pos + 4] == 'e') {
                p->pos += 5;
                p->current = JSON_TOK_FALSE;
            } else {
                p->current = JSON_TOK_ERROR;
                strncpy(p->error, "Invalid token", sizeof(p->error));
            }
            break;
        case 'n':
            if (p->pos + 4 <= p->len &&
                p->input[p->pos + 1] == 'u' &&
                p->input[p->pos + 2] == 'l' &&
                p->input[p->pos + 3] == 'l') {
                p->pos += 4;
                p->current = JSON_TOK_NULL;
            } else {
                p->current = JSON_TOK_ERROR;
                strncpy(p->error, "Invalid token", sizeof(p->error));
            }
            break;
        default:
            if (c == '-' || is_digit(c)) {
                p->current = parse_number(p);
            } else {
                p->current = JSON_TOK_ERROR;
                snprintf(p->error, sizeof(p->error), "Unexpected character: %c", c);
            }
            break;
    }

    return p->current;
}

// Peek current token
json_token_t json_peek_token(json_parser_t* p) {
    return p->current;
}

// Skip a value
int json_skip_value(json_parser_t* p) {
    switch (p->current) {
        case JSON_TOK_LBRACE: {
            // Skip object
            json_next_token(p);
            while (p->current != JSON_TOK_RBRACE && p->current != JSON_TOK_EOF) {
                if (p->current != JSON_TOK_STRING) return -1;
                json_next_token(p); // skip key
                if (p->current != JSON_TOK_COLON) return -1;
                json_next_token(p);
                if (json_skip_value(p) != 0) return -1;
                if (p->current == JSON_TOK_COMMA) {
                    json_next_token(p);
                }
            }
            if (p->current == JSON_TOK_RBRACE) {
                json_next_token(p);
            }
            return 0;
        }
        case JSON_TOK_LBRACKET: {
            // Skip array
            json_next_token(p);
            while (p->current != JSON_TOK_RBRACKET && p->current != JSON_TOK_EOF) {
                if (json_skip_value(p) != 0) return -1;
                if (p->current == JSON_TOK_COMMA) {
                    json_next_token(p);
                }
            }
            if (p->current == JSON_TOK_RBRACKET) {
                json_next_token(p);
            }
            return 0;
        }
        case JSON_TOK_STRING:
        case JSON_TOK_NUMBER:
        case JSON_TOK_TRUE:
        case JSON_TOK_FALSE:
        case JSON_TOK_NULL:
            json_next_token(p);
            return 0;
        default:
            return -1;
    }
}

// Parse string value
int json_parse_string(json_parser_t* p, char* buf, size_t size) {
    if (p->current != JSON_TOK_STRING) return -1;

    size_t copy_len = p->string_len < size - 1 ? p->string_len : size - 1;
    strncpy(buf, p->string_buf, copy_len);
    buf[copy_len] = '\0';

    json_next_token(p);
    return 0;
}

// Parse number value
int json_parse_number(json_parser_t* p, double* value) {
    if (p->current != JSON_TOK_NUMBER) return -1;

    *value = p->number_value;
    json_next_token(p);
    return 0;
}

// Parse integer value
int json_parse_int(json_parser_t* p, int* value) {
    if (p->current != JSON_TOK_NUMBER) return -1;

    *value = (int)p->number_value;
    json_next_token(p);
    return 0;
}

// Parse boolean value
int json_parse_bool(json_parser_t* p, bool* value) {
    if (p->current == JSON_TOK_TRUE) {
        *value = true;
        json_next_token(p);
        return 0;
    } else if (p->current == JSON_TOK_FALSE) {
        *value = false;
        json_next_token(p);
        return 0;
    }
    return -1;
}

// Expect a specific token
int json_expect(json_parser_t* p, json_token_t expected) {
    if (p->current != expected) return -1;
    json_next_token(p);
    return 0;
}

// ============================================================================
// JSON Writer Implementation
// ============================================================================

// Initialize writer
void json_writer_init(json_writer_t* w, char* buf, size_t size) {
    w->buf = buf;
    w->size = size;
    w->offset = 0;
    w->needs_comma = false;
    w->depth = 0;
    if (size > 0) {
        buf[0] = '\0';
    }
}

// Helper: write raw string
static int write_raw(json_writer_t* w, const char* str, size_t len) {
    if (w->offset + len >= w->size) return -1;
    strncpy(w->buf + w->offset, str, len);
    w->offset += len;
    w->buf[w->offset] = '\0';
    return 0;
}

// Helper: maybe write comma
static int maybe_comma(json_writer_t* w) {
    if (w->needs_comma) {
        if (write_raw(w, ",", 1) != 0) return -1;
    }
    return 0;
}

// Start object
int json_write_object_start(json_writer_t* w) {
    if (maybe_comma(w) != 0) return -1;
    if (write_raw(w, "{", 1) != 0) return -1;
    w->needs_comma = false;
    w->depth++;
    return 0;
}

// End object
int json_write_object_end(json_writer_t* w) {
    if (write_raw(w, "}", 1) != 0) return -1;
    w->needs_comma = true;
    w->depth--;
    return 0;
}

// Start array
int json_write_array_start(json_writer_t* w) {
    if (maybe_comma(w) != 0) return -1;
    if (write_raw(w, "[", 1) != 0) return -1;
    w->needs_comma = false;
    w->depth++;
    return 0;
}

// End array
int json_write_array_end(json_writer_t* w) {
    if (write_raw(w, "]", 1) != 0) return -1;
    w->needs_comma = true;
    w->depth--;
    return 0;
}

// Write key
int json_write_key(json_writer_t* w, const char* key) {
    if (maybe_comma(w) != 0) return -1;
    if (write_raw(w, "\"", 1) != 0) return -1;
    if (write_raw(w, key, strlen(key)) != 0) return -1;
    if (write_raw(w, "\":", 2) != 0) return -1;
    w->needs_comma = false;
    return 0;
}

// Write escaped string value
int json_write_string(json_writer_t* w, const char* value) {
    if (maybe_comma(w) != 0) return -1;
    if (write_raw(w, "\"", 1) != 0) return -1;

    // Escape special characters
    const char* p = value;
    while (*p) {
        char c = *p;
        const char* escaped = NULL;
        char buf[2] = {0, 0};

        switch (c) {
            case '"':  escaped = "\\\""; break;
            case '\\': escaped = "\\\\"; break;
            case '\b': escaped = "\\b"; break;
            case '\f': escaped = "\\f"; break;
            case '\n': escaped = "\\n"; break;
            case '\r': escaped = "\\r"; break;
            case '\t': escaped = "\\t"; break;
            default:
                if ((unsigned char)c < 32) {
                    // Skip other control characters
                    p++;
                    continue;
                }
                buf[0] = c;
                escaped = buf;
                break;
        }

        if (write_raw(w, escaped, strlen(escaped)) != 0) return -1;
        p++;
    }

    if (write_raw(w, "\"", 1) != 0) return -1;
    w->needs_comma = true;
    return 0;
}

// Write integer value
int json_write_int(json_writer_t* w, int value) {
    if (maybe_comma(w) != 0) return -1;

    char num[32];
    int len = snprintf(num, sizeof(num), "%d", value);
    if (len < 0 || (size_t)len >= sizeof(num)) return -1;

    if (write_raw(w, num, len) != 0) return -1;
    w->needs_comma = true;
    return 0;
}

// Write int64 value
int json_write_int64(json_writer_t* w, long long value) {
    if (maybe_comma(w) != 0) return -1;

    char num[32];
    int len = snprintf(num, sizeof(num), "%lld", value);
    if (len < 0 || (size_t)len >= sizeof(num)) return -1;

    if (write_raw(w, num, len) != 0) return -1;
    w->needs_comma = true;
    return 0;
}

// Write float value
int json_write_float(json_writer_t* w, float value) {
    if (maybe_comma(w) != 0) return -1;

    // Simple float to string (limited precision)
    char num[32];
    int intpart = (int)value;
    int fracpart = (int)((value - intpart) * 1000000);
    if (fracpart < 0) fracpart = -fracpart;

    int len;
    if (fracpart == 0) {
        len = snprintf(num, sizeof(num), "%d.0", intpart);
    } else {
        // Trim trailing zeros
        while (fracpart % 10 == 0 && fracpart > 0) {
            fracpart /= 10;
        }
        len = snprintf(num, sizeof(num), "%d.%d", intpart, fracpart);
    }

    if (len < 0 || (size_t)len >= sizeof(num)) return -1;

    if (write_raw(w, num, len) != 0) return -1;
    w->needs_comma = true;
    return 0;
}

// Write boolean value
int json_write_bool(json_writer_t* w, bool value) {
    if (maybe_comma(w) != 0) return -1;
    if (value) {
        if (write_raw(w, "true", 4) != 0) return -1;
    } else {
        if (write_raw(w, "false", 5) != 0) return -1;
    }
    w->needs_comma = true;
    return 0;
}

// Write null
int json_write_null(json_writer_t* w) {
    if (maybe_comma(w) != 0) return -1;
    if (write_raw(w, "null", 4) != 0) return -1;
    w->needs_comma = true;
    return 0;
}

// Key-value convenience functions
int json_write_kv_string(json_writer_t* w, const char* key, const char* value) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_string(w, value);
}

int json_write_kv_int(json_writer_t* w, const char* key, int value) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_int(w, value);
}

int json_write_kv_int64(json_writer_t* w, const char* key, long long value) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_int64(w, value);
}

int json_write_kv_float(json_writer_t* w, const char* key, float value) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_float(w, value);
}

int json_write_kv_bool(json_writer_t* w, const char* key, bool value) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_bool(w, value);
}

int json_write_kv_null(json_writer_t* w, const char* key) {
    if (json_write_key(w, key) != 0) return -1;
    return json_write_null(w);
}

// Write integer array
int json_write_int_array(json_writer_t* w, const int* arr, size_t count) {
    if (json_write_array_start(w) != 0) return -1;
    for (size_t i = 0; i < count; i++) {
        if (json_write_int(w, arr[i]) != 0) return -1;
    }
    return json_write_array_end(w);
}

// Write float array
int json_write_float_array(json_writer_t* w, const float* arr, size_t count) {
    if (json_write_array_start(w) != 0) return -1;
    for (size_t i = 0; i < count; i++) {
        if (json_write_float(w, arr[i]) != 0) return -1;
    }
    return json_write_array_end(w);
}

// Get current length
size_t json_writer_length(json_writer_t* w) {
    return w->offset;
}
