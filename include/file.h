#ifndef CTOML_FILE_H
#define CTOML_FILE_H

#include <stddef.h>

/** Functions */
int read_file(const char* filename, char** out_buf, size_t* out_size);
int check_toml_file(const char *filename);

#endif // CTOML_FILE_H