/*
 * json.h - Minimal JSON parser/writer for API request/response handling
 */

#ifndef JSON_H
#define JSON_H

#include <stddef.h>
#include <stdbool.h>

// JSON token types
typedef enum {
    JSON_TOK_LBRACE,    // {
    JSON_TOK_RBRACE,    // }
    JSON_TOK_LBRACKET,  // [
    JSON_TOK_RBRACKET,  // ]
    JSON_TOK_COLON,     // :
    JSON_TOK_COMMA,     // ,
    JSON_TOK_STRING,
    JSON_TOK_NUMBER,
    JSON_TOK_TRUE,
    JSON_TOK_FALSE,
    JSON_TOK_NULL,
    JSON_TOK_EOF,
    JSON_TOK_ERROR
} json_token_t;

// JSON parser state
typedef struct {
    const char* input;
    size_t len;
    size_t pos;
    json_token_t current;
    char string_buf[4096];
    size_t string_len;
    double number_value;
    char error[128];
} json_parser_t;

// Initialize parser
void json_parser_init(json_parser_t* p, const char* input, size_t len);

// Get next token
json_token_t json_next_token(json_parser_t* p);

// Peek current token without advancing
json_token_t json_peek_token(json_parser_t* p);

// Skip current value (object, array, or primitive)
int json_skip_value(json_parser_t* p);

// Parse a string value (current token must be JSON_TOK_STRING)
int json_parse_string(json_parser_t* p, char* buf, size_t size);

// Parse a number value (current token must be JSON_TOK_NUMBER)
int json_parse_number(json_parser_t* p, double* value);

// Parse an integer value
int json_parse_int(json_parser_t* p, int* value);

// Parse a boolean value
int json_parse_bool(json_parser_t* p, bool* value);

// Expect a specific token
int json_expect(json_parser_t* p, json_token_t expected);

// JSON Writer
typedef struct {
    char* buf;
    size_t size;
    size_t offset;
    bool needs_comma;
    int depth;
} json_writer_t;

// Initialize writer
void json_writer_init(json_writer_t* w, char* buf, size_t size);

// Object operations
int json_write_object_start(json_writer_t* w);
int json_write_object_end(json_writer_t* w);

// Array operations
int json_write_array_start(json_writer_t* w);
int json_write_array_end(json_writer_t* w);

// Write key (for objects)
int json_write_key(json_writer_t* w, const char* key);

// Write values
int json_write_string(json_writer_t* w, const char* value);
int json_write_int(json_writer_t* w, int value);
int json_write_int64(json_writer_t* w, long long value);
int json_write_float(json_writer_t* w, float value);
int json_write_bool(json_writer_t* w, bool value);
int json_write_null(json_writer_t* w);

// Write key-value pairs (convenience)
int json_write_kv_string(json_writer_t* w, const char* key, const char* value);
int json_write_kv_int(json_writer_t* w, const char* key, int value);
int json_write_kv_int64(json_writer_t* w, const char* key, long long value);
int json_write_kv_float(json_writer_t* w, const char* key, float value);
int json_write_kv_bool(json_writer_t* w, const char* key, bool value);
int json_write_kv_null(json_writer_t* w, const char* key);

// Write arrays of primitives
int json_write_int_array(json_writer_t* w, const int* arr, size_t count);
int json_write_float_array(json_writer_t* w, const float* arr, size_t count);

// Finalize and get length
size_t json_writer_length(json_writer_t* w);

#endif // JSON_H
