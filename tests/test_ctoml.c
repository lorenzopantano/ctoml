#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <file.h>
#include <ctype.h>
#include <lexer.h>

// TODO: USE THIS: https://github.com/toml-lang/toml-test

int main(int argc, char **argv) {

    // if (argc != 2) {
    //     fprintf(stderr, "Usage: %s <path_to_toml_file>\n", argv[0]);
    //     return EXIT_FAILURE;
    // }
    const char *filename = "../../res/test.toml";
    char *buf;
    size_t size;
    int result = read_file(filename, &buf, &size);
    if (result != 0) {
        fprintf(stderr, "Failed to read file %s\n", filename);
        return EXIT_FAILURE;
    }


    Lexer *l;
    lexer_init(l, buf, size);

    printf("Lexer src:\n%s\n\n", l->src);

    int count = 0;
    Token t;
    do {
        t = lexer_next_token(l);
        token_print(t);
        count++;
    } while (t.kind != TOK_EOF);

    free(buf);
    return 0;
}