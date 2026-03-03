#ifndef CTOM_STREAM_H
#define CTOM_STREAM_H

#include <stdlib.h>
#include <stdio.h>
#include <lexer.h>

/**
 * @brief Represents the Token stream emitted by the lexer
 * Just a utility struct to store tokens, instead of a raw array inside the lexer.
 */
typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
    size_t pos;
} TokenStream;

/** Functions */
TokenStream *stream_init(void);
void stream_push(TokenStream *s, Token t);
void stream_print(TokenStream *s);
void stream_advance(TokenStream *s);
Token stream_peek_n(TokenStream *s, int n);
Token stream_peek(TokenStream *s);

#endif // CTOM_STREAM_H