#include <stdlib.h>>
#include <stdio.h>

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
} TOMLType;

typedef struct TOMLValue {
    TOMLType type;
    union {
        char    *string;
        int64_t integer;
        double  float_;
        int     boolean;
        // TODO: Add arrays and tables here
    };
} TOMLValue;

typedef struct TOMLKeyValue {
    char        *key;
    TOMLValue   value;
} TOMLKeyValue;

/** Functions */
void parse_toml();
void parse_keyval();
void parse_key();
void parse_string();
void parse_integer();
void parse_float();
void parse_datetime();