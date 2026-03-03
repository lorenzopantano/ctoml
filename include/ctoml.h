#include "parser.h"

// Final TomlDoc product
typedef struct TomlDoc {
    TomlKeyValue    *doc;
    int             n_items;
} TomlDoc;

char * toml_get_string(TomlDoc *doc, char *name);
int toml_get_int(TomlDoc *doc, char *name);
float toml_get_float(TomlDoc *doc, char *name);