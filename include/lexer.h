#ifndef CTOML_LEXER_H
#define CTOML_LEXER_H

#include <stdlib.h>
#include "error.h"

/**
 * @brief Defines possible token kinds emitted 
 * by the Lexer.
 */
typedef enum {
    TOK_EOF,
    TOK_NEWLINE,       // \n or \r\n
    TOK_COMMENT,       // # ...
    TOK_BARE_KEY,      // [A-Za-z0-9_-]+
    TOK_EQUALS,        // =
    TOK_DOT,           // .
    TOK_COMMA,         // ,
    TOK_LBRACKET,      // [
    TOK_RBRACKET,      // ]
    TOK_DOUBLE_LBRACKET, // [[
    TOK_DOUBLE_RBRACKET, // ]]
    TOK_LBRACE,        // {
    TOK_RBRACE,        // }
    TOK_STRING,        // "..." or '...' or """...""" or '''...'''
    TOK_INTEGER,       // 42, 0xFF, 0o77, 0b1101
    TOK_FLOAT,         // 3.14, 1e10, inf, nan
    TOK_BOOLEAN,       // true | false
    TOK_OFFSET_DATETIME,   // 1979-05-27T07:32:00Z
    TOK_LOCAL_DATETIME,    // 1979-05-27T07:32:00
    TOK_LOCAL_DATE,        // 1979-05-27
    TOK_LOCAL_TIME,        // 07:32:00
    TOK_INVALID,        // Invalid tokens
    TOK_ERROR           // Invalid/Error tokens (only from parser)
} TokenKind;

/**
 * @brief Describes a Token, represented by:
 * The kind of the token, the raw literal value from source, the line marker in the source file,
 * the col marker in the source file, the pointer to the start of the token in the source,
 * the lenght of the pointer from start.
 */
typedef struct {
    TokenKind kind;
    char        *lit;     // raw literal from source
    int         line;
    int         col;
    const char  *start;
    size_t      len;
} Token;

/**
 * @brief Lexer struct, stores source buffer, current and end pointers for iteration
 * and line and col markers to the source file.
 */
typedef struct {
    const char *src;      // full source (from read_file)
    const char *current;  // pointer to next unread char
    const char *end;      // src + file_size
    int         line;
    int         col;
    TomlError  *err;
    int         had_error; // Prevents error overwrites
} Lexer;

static const char *token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOK_EOF:              return "EOF";
        case TOK_NEWLINE:          return "NEWLINE";
        case TOK_EQUALS:           return "EQUALS";
        case TOK_DOT:              return "DOT";
        case TOK_COMMA:            return "COMMA";
        case TOK_LBRACKET:         return "LBRACKET";
        case TOK_RBRACKET:         return "RBRACKET";
        case TOK_DOUBLE_LBRACKET:  return "DOUBLE_LBRACKET";
        case TOK_DOUBLE_RBRACKET:  return "DOUBLE_RBRACKET";
        case TOK_LBRACE:           return "LBRACE";
        case TOK_RBRACE:           return "RBRACE";
        case TOK_BARE_KEY:         return "BARE_KEY";
        case TOK_STRING:           return "STRING";
        case TOK_INTEGER:          return "INTEGER";
        case TOK_FLOAT:            return "FLOAT";
        case TOK_BOOLEAN:          return "BOOLEAN";
        case TOK_COMMENT:          return "COMMENT";
        case TOK_LOCAL_DATE:       return "LOCAL_DATE";
        case TOK_LOCAL_DATETIME:   return "LOCAL_DATETIME";
        case TOK_OFFSET_DATETIME:  return "OFFSET_DATETIME";
        case TOK_LOCAL_TIME:       return "LOCAL_TIME";
        case TOK_INVALID:          return "INVALID";
        default:                   return "UNKNOWN";
    }
}

/** Functions */
void lexer_init(Lexer *l, const char *src, size_t len);
void lexer_advance(Lexer *l);
char lexer_peek(Lexer *l);
char lexer_peek_n(Lexer *l, int n);
void lexer_skip_whitespace(Lexer *l);
void token_print(Token t);
Token lexer_emit_error(Lexer *l, TomlErrorCode code,char *msg, const char *start);
Token lexer_emit_token(Lexer *l, TokenKind kind, const char *start);
Token lexer_next_token(Lexer *l);
Token lexer_scan_brackets(Lexer *l, const char *start, TokenKind singleKind, TokenKind doubleKind);
Token lexer_scan_basic_string(Lexer *l);
Token lexer_scan_multiline_string(Lexer *l);
Token lexer_scan_multiline_literal_string(Lexer *l);
Token lexer_scan_literal_string(Lexer *l);

#endif // CTOML_LEXER_H