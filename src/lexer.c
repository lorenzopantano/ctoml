#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

/**
 * Initializes a lexer instance with the given source code.
 *
 * @param l Pointer to the Lexer structure to initialize
 * @param src Pointer to the source code string
 * @param len Length of the source code in bytes
 *
 * @note The source code is not copied; the lexer maintains pointers to the original string.
 *       Ensure the source remains valid for the lexer's lifetime.
 */
void lexer_init(Lexer *l, const char *src, size_t len) {
    l->src = src;
    l->current = src;
    l->end = src + len;
    l->line = 1;
    l->col = 1;
}

/**
 * Advances the lexer's current position to the next character.
 * 
 * Updates line and column tracking based on the current character:
 * - If the current character is a newline, increments the line counter
 *   and resets the column counter to 1
 * - Otherwise, increments the column counter
 * 
 * Always advances the current pointer to the next character.
 * 
 * @param l Pointer to the Lexer structure to advance
 */
void lexer_advance(Lexer *l) {
    // When new line, reset line and col
    if (*l->current == '\n') { l->line++; l->col = 1; } 
    // Otherwise only col
    else { l->col++; }
    // Either case, advance current pointer
    l->current++;
}

/**
 * @brief Peeks at the current character in the lexer without advancing the position.
 * 
 * Returns the character at the current position in the input stream without
 * consuming it. If the lexer has reached the end of the input, EOF is returned.
 * 
 * @param l Pointer to the Lexer structure
 * @return The current character, or EOF if at end of input
 */
char lexer_peek(Lexer *l) {
    if (l->current >= l->end) return EOF;
    else return *(l->current);
}

/**
 * @brief Same as lexer_peek but peeks 1 char ahead, without consuming it.
 * 
 * @param l Pointer to the Lexer structure
 * @return The char at position current + 1 if any
 */
char lexer_peek_2(Lexer *l) {
    if (l->current + 1 >= l->end) return EOF;
    else return *(l->current + 1);
}

/**
 * @brief Advances in the Lexer while characters are whitespaces (' ' or '\t')
 * 
 * @param l Pointer to the Lexer structure
 */
void lexer_skip_whitespace(Lexer *l) {
    while (lexer_peek(l) == ' ' || lexer_peek(l) == '\t') {
        lexer_advance(l);
    }
}


/// TOKENS

/**
 * @brief Emits a new token
 * 
 * @param l Pointer to the Lexer structure
 * @param kind Token kind to be emitted
 * @param start Pointer to start position for the new token
 * @return Token 
 */
Token lexer_emit_token(Lexer *l, TokenKind kind, const char *start) {
    return (Token) {
        .kind = kind,
        .start = start,
        .len = (size_t)(l->current - start), // Because of this, always do lexer_advance BEFORE emitting token
        .line = l->line,
        .col = l->col,
    };
}

/**
 * @brief Debug helper to print a Token struct
 * 
 * @param t Token
 */
void token_print(Token t) {
    printf("[%-16s] line=%-3d col=%-3d len=%-3zu lit=\"%.*s\"\n",
        token_kind_name(t.kind),
        t.line,
        t.col,
        t.len,
        (int)t.len, t.start   // %.*s: length first, then pointer
    );
}

/**
 * @brief Scan for square brackets tokens: [ ] or [[ ]]
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_brackets(Lexer *l, const char *start, TokenKind singleKind, TokenKind doubleKind) {
    lexer_advance(l);
    if (
        singleKind == TOK_LBRACKET && lexer_peek(l) == '[' || 
        singleKind == TOK_RBRACKET && lexer_peek(l) == ']'
    ) {
        lexer_advance(l);
        return lexer_emit_token(l, doubleKind, start);
    } 
    return lexer_emit_token(l, singleKind, start);
}


/**
 * @brief Scans the current lexer position (skipping whitespaces) for the next
 * token to emit
 * 
 * @param l Pointer to the lexer structure
 * @return Token 
 */
Token lexer_next_token(Lexer *l) {
    lexer_skip_whitespace(l);
    const char *start = l->current;
    char ch = lexer_peek(l);
    
    // Special cases
    if (ch == EOF) { return lexer_emit_token(l, TOK_EOF, start); }
    if (ch == '\n') {  lexer_advance(l); return lexer_emit_token(l, TOK_NEWLINE, start); }
    // Shouldn't be needed because of read file with 'rb' (TODO: double check this)
    if (ch == '\r' && lexer_peek_2(l) == '\n') { 
        lexer_advance(l); 
        lexer_advance(l); 
        return lexer_emit_token(l, TOK_NEWLINE, start);
    }

    // Single char tokens
    if (ch == '=') { lexer_advance(l); return lexer_emit_token(l, TOK_EQUALS, start); }
    if (ch == '.') { lexer_advance(l); return lexer_emit_token(l, TOK_DOT, start); }
    if (ch == ',') { lexer_advance(l); return lexer_emit_token(l, TOK_COMMA, start); }
    if (ch == '{') { lexer_advance(l); return lexer_emit_token(l, TOK_LBRACE, start); }
    if (ch == '}') { lexer_advance(l); return lexer_emit_token(l, TOK_RBRACE, start); }

    // Square brackets tokens
    if (ch == '[') { return lexer_scan_brackets(l, start, TOK_LBRACKET, TOK_DOUBLE_LBRACKET); }
    if (ch == ']') { return lexer_scan_brackets(l, start, TOK_RBRACKET, TOK_DOUBLE_RBRACKET); }

    lexer_advance(l);
    return lexer_emit_token(l, TOK_INVALID, start);
}