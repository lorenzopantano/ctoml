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
    if (l->current >= l->end) return '\0';
    else return *(l->current);
}

/**
 * @brief Same as lexer_peek but peeks 1 char ahead, without consuming it.
 * 
 * @param l Pointer to the Lexer structure
 * @return The char at position current + 1 if any
 */
char lexer_peek_2(Lexer *l) {
    if (l->current + 1 >= l->end) return '\0';
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

/**
 * @brief Advances in the lexer while characters are unicode escapes. TOML accepts
 * as valid only Unicode scalars, this excludes Unicode surrogates.
 * 
 * @param l Pointer to the Lexer structure
 * @param count How many characters to skip: \xHH, \uHHHH, \UHHHHHHHH
 * @return int 0 for success, 1 for failure
 */
static int lexer_skip_unicode_escape(Lexer *l, int count) {
    uint32_t codepoint = 0;

    for (int i = 0; i < count; i++) {
        char ch = lexer_peek(l);
        if (!isxdigit((unsigned char)ch)) return 0;

        // build the codepoint value as we go
        codepoint <<= 4;
        if      (ch >= '0' && ch <= '9') codepoint += ch - '0';
        else if (ch >= 'a' && ch <= 'f') codepoint += ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') codepoint += ch - 'A' + 10;

        lexer_advance(l);
    }

    // Reject Unicode surrogates
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return 0;
    // Reject values beyond Unicode range (only relevant for \U)
    if (codepoint > 0x10FFFF) return 0;

    return 1;
}


/**
 * @brief Consume and validate one escape sequence after the leading backslash.
 * The backslash itself must already be consumed before calling this.
 * 
 * @param l Pointer to the Lexer structure
 * @return int 0 for success, 1 for failure
 */
static int lexer_skip_escaped(Lexer *l) {
    char esc = lexer_peek(l);
    switch (esc) {
        case 'b': case 't': case 'n': case 'f':
        case 'r': case '"': case '\\': case 'e':
            lexer_advance(l);
            return 1;
        case 'x':
            lexer_advance(l);
            return lexer_skip_unicode_escape(l, 2);
        case 'u':
            lexer_advance(l);
            return lexer_skip_unicode_escape(l, 4);
        case 'U':
            lexer_advance(l);
            return lexer_skip_unicode_escape(l, 8);
        default:
            return 0;
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
 * @brief Scan for basic string tokens, basic strings are those wrapped in double quotes.
 * Reference -> https://toml.io/en/v1.1.0#string
 * 
 * @param l Pointer to the Lexer structure
 * @param start Start pointer
 * @return Token 
 */
Token lexer_scan_basic_string(Lexer *l) {
    lexer_advance(l); // Consume starting double quote first
    const char *start = l->current;
    while (1) { 
        char ch = lexer_peek(l);

        // Not allowed chars in basic strings: EOF or newlines, if we encounter them before closing double quote, it's an error
        if (ch == '\0' || ch == '\n' || ch == '\r') { break; }

        // Escaped chars
        if (ch == '\\') {
            lexer_advance(l); // Consume first backslash
            if (!lexer_skip_escaped(l)) { return lexer_emit_token(l, TOK_INVALID, start); }
        }

        // If we encounter a closing double quote, emit the string token and consume the closing double quote before returning
        if (ch == '"') { 
            // Advance before returning to consume the closing double quote
            Token t = lexer_emit_token(l, TOK_STRING, start);
            lexer_advance(l); 
            return t;
        }
        else { lexer_advance(l); }    
    }

    // TODO: Should throw an error here (EOF or NEWLINE reached before string end double quote)
    return lexer_emit_token(l, TOK_INVALID, start);
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
    if (ch == '\0') { return lexer_emit_token(l, TOK_EOF, start); }
    if (ch == '\n') {  lexer_advance(l); return lexer_emit_token(l, TOK_NEWLINE, start); }

    // Single char tokens
    if (ch == '=') { lexer_advance(l); return lexer_emit_token(l, TOK_EQUALS, start); }
    if (ch == '.') { lexer_advance(l); return lexer_emit_token(l, TOK_DOT, start); }
    if (ch == ',') { lexer_advance(l); return lexer_emit_token(l, TOK_COMMA, start); }
    if (ch == '{') { lexer_advance(l); return lexer_emit_token(l, TOK_LBRACE, start); }
    if (ch == '}') { lexer_advance(l); return lexer_emit_token(l, TOK_RBRACE, start); }

    // Square brackets tokens
    if (ch == '[') { return lexer_scan_brackets(l, start, TOK_LBRACKET, TOK_DOUBLE_LBRACKET); }
    if (ch == ']') { return lexer_scan_brackets(l, start, TOK_RBRACKET, TOK_DOUBLE_RBRACKET); }

    // Strings
    if (ch == '"') { return lexer_scan_basic_string(l); }

    lexer_advance(l);
    return lexer_emit_token(l, TOK_INVALID, start);
}