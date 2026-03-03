#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "stream.h"

void parse_toml(TokenStream *s, Lexer *l) {
    Token current = stream_peek(s);
    while (current.kind != TOK_EOF) {

        token_print(current);

        // Lexer errors
        if (current.kind == TOK_INVALID) {
            if (l->had_error) {
                print_error(l->err);
                return;
            }
        }

        stream_advance(s);
        current = stream_peek(s);
    }
    return;
};

void parse_keyval() {};

void parse_string() {}