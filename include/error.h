#ifndef CTOML_ERROR_H
#define CTOML_ERROR_H

typedef enum {
    TOML_OK = 0,

    // Lexer errors (character-level)
    TOML_ERR_INVALID_CHAR,        // bare \r, \x00
    TOML_ERR_INVALID_ESCAPE,      // \q, \uDFFF
    TOML_ERR_UNTERMINATED_STRING, // "hello at EOF
    TOML_ERR_INVALID_NUMBER,      // 23abc, 1__0

    // Parser errors (sequence-level)
    TOML_ERR_INVALID_VALUE,       // key = .7
    TOML_ERR_MISSING_EQUALS,      // key value
    TOML_ERR_DUPLICATE_KEY,       // key = 1 then key = 1
    TOML_ERR_MALFORMED_TABLE,     // [[key]
    TOML_ERR_UNEXPECTED_TOKEN,    // Generic parser error
    TOML_ERR_UNEXPECTED_EOF,      // Parser ran out of tokens
} TomlErrorCode;

typedef enum {
    TOML_ERR_SOURCE_NONE = 0,
    TOML_ERR_SOURCE_LEXER,
    TOML_ERR_SOURCE_PARSER,
} TomlErrorSource;

typedef struct {
    TomlErrorCode   code;          // Machine-readable (switch/if)
    TomlErrorSource source;        // WHO caught it: lexer or parser
    int             line;          // 1-based line number
    int             col;           // 1-based column number
    char            *msg;          // Human-readable description
    char            context[64];   // Snippet of offending input
} TomlError;


void print_error(TomlError *err);

#endif