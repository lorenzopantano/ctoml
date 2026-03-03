// #include <ctoml.h>

// FINAL LIBRARY USAGE EXAMPLE
// int main() {
//     // 1. Parse the file
//     TomlError err;
//     TomlDoc* doc = toml_parse_file("config.toml", &err);
//     if (!doc) {
//         fprintf(stderr, "Parse error: %s at line %d\n", err.msg, err.line);
//         return 1;
//     }

//     // 2. Read values (type-safe)
//     const char* title  = toml_get_string(doc, "title");
//     int         port   = toml_get_int(doc, "server.port");  // Dotted key
//     double      scale  = toml_get_float(doc, "scale");
//     bool        debug  = toml_get_bool(doc, "debug");

//     // 3. Tables & arrays
//     TomlTable* server  = toml_get_table(doc, "server");
//     const char* host   = toml_get_string(server, "host");

//     TomlArray* tags    = toml_get_array(doc, "tags");
//     for (int i = 0; i < toml_array_len(tags); i++) {
//         printf("tag: %s\n", toml_array_get_string(tags, i));
//     }

//     // 4. Always free
//     toml_free(doc);
//     return 0;
// }
