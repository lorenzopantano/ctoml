#include <stdlib.h>
#include <stdio.h>
#include "stream.h"
#include "lexer.h"

/**
 * @brief Initialize a token stream
 * 
 * @return TokenStream* 
 */
TokenStream *stream_init(void) {
    Token *tokens = malloc(sizeof(Token));
    TokenStream *s = malloc(sizeof(TokenStream));
    s->pos = 0;
    s->count = 0;
    s->tokens = tokens;
    s->capacity = 1;
    return s;
}

/**
 * @brief Push a new token to the token stream
 * 
 * @param s Pointer to the TokenStream
 * @param t Token to push
 */
void stream_push(TokenStream *s, Token t) {
    if (s->count >= s->capacity) {
        s->capacity++;
        // Add memory to the pointer array
        s->tokens = realloc(s->tokens, sizeof(Token)*s->capacity);
    }
    s->tokens[s->count++] = t;
}

/**
 * @brief Prints the TokenStream (for debug)
 * 
 * @param s Pointer to the TokenStream
 */
void stream_print(TokenStream *s) {
    printf("TokenStream of count: %zu, capacity: %zu\n", s->count, s->capacity);
    for (int i = 0; i < s->count; i++) {
        token_print(s->tokens[i]);
    }
}

/**
 * @brief Peeks n tokens inside the stream (if available)
 * 
 * @param s Pointer to the TokenStream
 * @param n Number of tokens to peek
 * @returns Peeked Token
 */
Token stream_peek_n(TokenStream *s, int n) {
    if (s->pos + n >= s->count) { return (Token) { .kind = TOK_EOF }; } // This is an error
    return s->tokens[s->pos + n];
}
Token stream_peek(TokenStream *s) { return stream_peek_n(s, 0); }

/**
 * @brief Advances in the TokenStream
 * 
 * @param s Pointer to the TokenStream
 */
void stream_advance(TokenStream *s) { s->pos++; }

