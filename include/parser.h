#ifndef CTOML_PARSER_H
#define CTOML_PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include "stream.h"

typedef enum {
    TOML_STRING,
    TOML_INTEGER,
    TOML_FLOAT,
    TOML_BOOL,
    TOML_LOCAL_DATE,
    TOML_LOCAL_TIME,
    TOML_LOCAL_DATETIME,
    TOML_OFFSET_DATETIME,
    TOML_ARRAY,
    TOML_TABLE,
    TOML_INLINE_TABLE
} TomlType;

typedef struct TomlValue {
    TomlType type;
    union {
        char    *string;
        int64_t integer;
        double  float_;
        int     boolean;
        // TODO: Add arrays and tables here
    };
} TomlValue;

typedef struct TomlKeyValue {
    char        *key;
    TomlValue   value;
} TomlKeyValue;


/** Functions */
void parse_toml(TokenStream *s, Lexer *l);
void parse_keyval();
void parse_key();
void parse_string();
void parse_integer();
void parse_float();
void parse_datetime();

#endif // CTOML_PARSER_H