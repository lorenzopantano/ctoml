#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
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
 * @brief Advances the lexer n characters, updating line and column tracking accordingly.
 * 
 * @param l Pointer to the Lexer structure to advance
 * @param n Number of characters to advance
 */
void lexer_advance_n(Lexer *l, int n) {
    for (int i = 0; i < n; i++) {
        lexer_advance(l);
    }
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
 * @brief Same as lexer_peek but peeks n chars ahead, without consuming it.
 * 
 * @param l Pointer to the Lexer structure
 * @return The char at position current + n if any
 */
char lexer_peek_n(Lexer *l, int n) {
    if (l->current + n >= l->end) return '\0';
    else return *(l->current + n);
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
 * @brief Advances in the Lexer while characters are whitespaces, incluing newlines
 * (used in the multiline string \ special character)
 * 
 * @param l Pointer to the Lexer structure
 */
void lexer_skip_whitespace_and_newlines(Lexer *l) {
    while (lexer_peek(l) == ' '  || lexer_peek(l) == '\t' ||
           lexer_peek(l) == '\n' || lexer_peek(l) == '\r')
        lexer_advance(l);
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

/**
 * @brief Check if current lexer position buffer matches exactly 
 * the expected buffer, given the lenght
 * 
 * @param l Pointer to the Lexer structure
 * @param expected The expected buffer to match against
 * @param len Size of the expected buffer
 * @return int 
 */
static int lexer_match(Lexer *l, const char *expected, size_t len) {
    if (l->current + len > l->end) return 0;
    return strncmp(l->current, expected, len) == 0;
}


/**
 * @brief Checks if the given character is a valid bare key character (alphanumeric, - or _)
 * 
 * @param ch The character to check
 * @return int 1 if the character is a valid bare key character, 0 otherwise
 */
static int is_bare_key_char(char ch) {
    return isalnum((unsigned char)ch) || ch == '-' || ch == '_';
}

/**
 * @brief Checks if the given character is a valid number character (digit, - or _)
 * 
 * @param ch The character to check
 * @return int 1 if the character is a valid number character, 0 otherwise
 */
static int is_number_char(char ch) {
    return isdigit((unsigned char)ch) || ch == '-' || ch == '_';
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
 * @brief Same as lexer_emit_token but let's you specify an end pointer.
 * 
 * @param l Pointer to the Lexer structure
 * @param kind Token kind to be emitted
 * @param start Pointer to start position for the new token
 * @return Token 
 * @return Token 
 */
Token lexer_emit_token_end(Lexer *l, TokenKind kind, const char *start, const char *end) {
    return (Token){
        .kind  = kind,
        .start = start,
        .len   = (size_t)(end - start),  // ← custom end, not l->current
        .line  = l->line,
        .col   = l->col,
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
 * @brief Scan for multiline string tokens
 * Reference -> https://toml.io/en/v1.1.0#string
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_multiline_string(Lexer *l) {
    lexer_advance(l);
    lexer_advance(l);
    lexer_advance(l);  // Consume """

    // According to v1.1.0 docs, newlines after opening quotes will be trimmed.
    if (lexer_peek(l) == '\n') {
        lexer_advance(l);
    } else if (lexer_peek(l) == '\r' && lexer_peek_n(l, 1) == '\n') {
        lexer_advance(l); lexer_advance(l);
    }

    const char *start = l->current;

    while (1) {
        char ch = lexer_peek(l);

        if (ch == '\0') { return lexer_emit_token(l, TOK_INVALID, start); }

        // From docs: 
        // For writing long strings without introducing extraneous whitespace, 
        // use a "line ending backslash". 
        // When the last non-whitespace character on a line is an unescaped \, 
        // it will be trimmed along with all whitespace (including newlines) 
        // up to the next non-whitespace character or closing delimiter.
        if (ch == '\\') {
            lexer_advance(l); 
            char next = lexer_peek(l);

            // If we encounter EOF right after the backslash, it's an error
            if (next == '\0') { return lexer_emit_token(l, TOK_INVALID, start); }

            if (next == ' ' || next == '\t' || next == '\n' || next == '\r') {
                lexer_skip_whitespace_and_newlines(l);
                continue;
            }

            if (!lexer_skip_escaped(l)) { return lexer_emit_token(l, TOK_INVALID, start); }
            continue;
        }

        // Bare \r only valid before \n, if we encounter it alone it's an error
        if (ch == '\r') {
            if (lexer_peek_n(l, 1) != '\n') { return lexer_emit_token(l, TOK_INVALID, start); }
            lexer_advance(l); lexer_advance(l);
            continue;
        }

        // End of multiline string
        if (ch == '"') {
            // Consider edge case where double quotes are put just before the three closing ones.
            // max 2
            int count = 0;
            const char *quote_start = l->current;
            while (lexer_peek(l) == '"' && count < 5) {
                lexer_advance(l);
                count++;
            }
            if (count >= 3)
                return lexer_emit_token_end(l, TOK_STRING, start, quote_start + (count - 3));
            continue;
        }

        // If this character is a control character ( <= 0x1F  or  == 0x7F ), AND 
        // it is not one of the explicitly allowed ones (\t or \n), 
        // then it’s invalid
        if (ch != '\t' && ch != '\n' &&  ((unsigned char)ch <= 0x1F || (unsigned char)ch == 0x7F)) {
            return lexer_emit_token(l, TOK_INVALID, start);
        }

        lexer_advance(l);
    }
}

/**
 * @brief Scans for literal string tokens, literal strings are those wrapped in single quotes.
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_literal_string(Lexer *l) {
    lexer_advance(l);
    const char *start = l->current;
    while (1) {
        char ch = lexer_peek(l);
        if (ch == '\0') { return lexer_emit_token(l, TOK_INVALID, start); }

        // Newlines are not allowed in literal strings
        if (ch == '\n' || ch == '\r') { return lexer_emit_token(l, TOK_INVALID, start); }

        // Control characters other than tab are not permitted in a literal string.
        // The unsiged char check inlucdes the \n and \r check above, 
        // but we check them explictly to give a more specific error message.
        if (ch != '\t' && ((unsigned char)ch <= 0x1F || (unsigned char)ch == 0x7F)) { return lexer_emit_token(l, TOK_INVALID, start); }
        
        // End of literal string
        if (ch == '\'') break;
        else lexer_advance(l); 
    }
    Token t = lexer_emit_token(l, TOK_STRING, start); 
    lexer_advance(l);
    return t;
}

/**
 * @brief Scan for multiline literal string tokens, multiline literal strings are those wrapped in triple single quotes.
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_multiline_literal_string(Lexer *l) {
    lexer_advance(l);
    lexer_advance(l);
    lexer_advance(l);  // Consume '''

    // Trim immediate first newline
    if (lexer_peek(l) == '\n') {
        lexer_advance(l);
    } else if (lexer_peek(l) == '\r' && lexer_peek_n(l, 1) == '\n') {
        lexer_advance(l); lexer_advance(l);
    }

    const char *start = l->current;

    while (1) {
        char ch = lexer_peek(l);

        // Unterminated
        if (ch == '\0') { return lexer_emit_token(l, TOK_INVALID, start); }

        // Closing ''' with 4/5 quote edge case (same logic as multiline basic)
        if (ch == '\'') {
            int count = 0;
            const char *quote_start = l->current;
            while (lexer_peek(l) == '\'' && count < 5) {
                lexer_advance(l);
                count++;
            }
            if (count >= 3)
                return lexer_emit_token_end(l, TOK_STRING, start, quote_start + (count - 3));
            continue;  // 1 or 2 quotes = content, keep going
        }

        // Bare \r only valid before \n
        if (ch == '\r') {
            if (lexer_peek_n(l, 1) != '\n') { return lexer_emit_token(l, TOK_INVALID, start); }
            lexer_advance(l); lexer_advance(l);
            continue;
        }

        // Control characters other than tab are not permitted
        // \n is allowed raw (multiline), \t is allowed raw (spec)
        if (ch != '\t' && ch != '\n' && ((unsigned char)ch <= 0x1F || (unsigned char)ch == 0x7F))
            return lexer_emit_token(l, TOK_INVALID, start);

        lexer_advance(l);
    }
}

/**
 * @brief Scan for bare keys. Bare keys are only composed by ascii characters a-Z,A-Z,0-9,_,-
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_bare_key(Lexer *l) {
    const char *start = l->current;
    while (1) {
        char ch = lexer_peek(l);
        if (is_bare_key_char(ch)) { lexer_advance(l); continue; }
        else break;
    }
    return lexer_emit_token(l, TOK_BARE_KEY, start);
}

/**
 * @brief Scan for comment tokens, comments start with # and go until the end of the line (newline or EOF)
 * Control characters other than tab are not allowed in comments
 * 
 * @param l 
 * @return Token 
 */
Token lexer_scan_comment(Lexer *l) {
        const char *start = l->current;
        while (1) {
            char ch = lexer_peek(l);
            if (ch == '\0' || ch == '\n' || ch == '\r') break;
            else if (ch != '\t' && ((unsigned char)ch <= 0x1F || (unsigned char)ch == 0x7F)) { return lexer_emit_token(l, TOK_INVALID, start); }
            else lexer_advance(l);
        }
    return lexer_emit_token(l, TOK_COMMENT, start);
}

/**
 * @brief Scan for number tokens, including integers, floats, inf and nan.
 * 
 * @param l Pointer to the Lexer structure
 * @return Token 
 */
Token lexer_scan_number(Lexer *l) {
    const char *start = l->current;
    int has_minus = 0;

    // Optional leading sign
    if (lexer_peek(l) == '+') { lexer_advance(l); }
    if (lexer_peek(l) == '-') { has_minus = 1; lexer_advance(l); }

    // Check for inf and nan first, since they can start with a digit or a sign
    if (lexer_match(l, "inf", 3) && !is_bare_key_char(*(l->current + 3))) {
        lexer_advance_n(l, 3);
        return lexer_emit_token(l, TOK_FLOAT, start);
    }
    if (lexer_match(l, "nan", 3) && !is_bare_key_char(*(l->current + 3))) {
        lexer_advance_n(l, 3);
        return lexer_emit_token(l, TOK_FLOAT, start);
    }

    // Integers
    char ch = lexer_peek(l);
    // Leading zeros not allowed
    if (ch == '0' && is_number_char(lexer_peek_n(l, 1))) {  return lexer_emit_token(l, TOK_INVALID, start); }

    while (is_number_char(ch)) { 
        lexer_advance(l); 
        ch = lexer_peek(l);
    }
    return lexer_emit_token(l, TOK_INTEGER, start);
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
    if (ch == '\r' && lexer_peek_n(l, 1) == '\n') { 
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
    if (ch == '#') { return lexer_scan_comment(l); }

    // Square brackets tokens
    if (ch == '[') { return lexer_scan_brackets(l, start, TOK_LBRACKET, TOK_DOUBLE_LBRACKET); }
    if (ch == ']') { return lexer_scan_brackets(l, start, TOK_RBRACKET, TOK_DOUBLE_RBRACKET); }

    // Strings
    if (ch == '"' && lexer_peek_n(l, 1) == '"' && lexer_peek_n(l, 2) == '"') { return lexer_scan_multiline_string(l); }
    if (ch == '"') { return lexer_scan_basic_string(l); }
    if (ch == '\'' && lexer_peek_n(l, 1) == '\'' && lexer_peek_n(l, 2) == '\'') { return lexer_scan_multiline_literal_string(l); }
    if (ch == '\'') { return lexer_scan_literal_string(l); }

    // Booleans
    if (lexer_match(l, "true", 4) && !is_bare_key_char(*(l->current + 4))) {
        lexer_advance_n(l, 4);
        return lexer_emit_token(l, TOK_BOOLEAN, start);
    }
    if (lexer_match(l, "false", 5) && !is_bare_key_char(*(l->current + 5))) {
        lexer_advance_n(l, 5);
        return lexer_emit_token(l, TOK_BOOLEAN, start);
    }

    // Numbers (integers, floats, inf, nan) // TODO: Add dates here
    if (lexer_match(l, "inf", 3) && !is_bare_key_char(*(l->current + 3))) {
        lexer_advance_n(l, 3);
        return lexer_emit_token(l, TOK_FLOAT, start);
    }
    if (lexer_match(l, "nan", 3) && !is_bare_key_char(*(l->current + 3))) {
        lexer_advance_n(l, 3);
        return lexer_emit_token(l, TOK_FLOAT, start);
    }
    if (isdigit((unsigned char)ch) || ch == '-' || ch == '+') { return lexer_scan_number(l); }

    // Bare keys
    if (is_bare_key_char(ch)) { return lexer_scan_bare_key(l); }

    lexer_advance(l);
    return lexer_emit_token(l, TOK_INVALID, start);
}