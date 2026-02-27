#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int read_file(const char* filename, char** out_buf, size_t* out_size) {
    // Open the file in binary mode to avoid issues with line endings on different platforms:
    // OS messes with line endings (\r\n or \n) and this can cause issues when parsing the file
    // so we want to read the file as binary and handle line endings ourselves.
    FILE *f = fopen(filename, "rb");
    if (!f) { perror("Failed to open file"); return EXIT_FAILURE; }


    // Set cursor to end of file, read file size, and set back cursor to start of file
    if (fseek(f, 0, SEEK_END) != 0) { perror("fseek"); return EXIT_FAILURE; }
    long sz = ftell(f);
    if (sz < 0) { perror("ftell"); return EXIT_FAILURE; }
    if (fseek(f, 0, SEEK_SET) != 0) { perror("fseek - set"); return EXIT_FAILURE; }

    // Read file into buffer
    char *buf = malloc(sz + 1);
    if (!buf) { perror("malloc"); return EXIT_FAILURE; }

    size_t read_sz = fread(buf, 1, sz, f);
    if (read_sz != (size_t)sz) {
        fprintf(stderr, "Failed to read file: expected %ld bytes, got %zu bytes\n", sz, read_sz);
        free(buf);
        return EXIT_FAILURE;
    }
    buf[sz] = '\0'; // Null-terminate the buffer
    fclose(f);
    
    *out_buf = buf;
    *out_size = sz;
    return 0;
}