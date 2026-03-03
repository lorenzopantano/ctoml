#include <error.h>
#include <stdio.h>
#include <stdlib.h>

void print_error(TomlError *err) {
    printf("ERROR: %s at [%d:%d] - %.*s\n", err->msg, err->line, err->col, (int)sizeof(err->context), err->context);
}