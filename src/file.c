#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * @brief Checks for filename extension, must be ".toml"
 * 
 * @param filename Path to the filename
 * @return int 0 if ext is .toml
 */
int check_toml_file(const char *filename) {
    const char* dot = strrchr(filename, '.');  // Last '.' in path
    if (!dot) return EXIT_FAILURE;
    return strcmp(dot, ".toml");
}

/**
 * @brief Reads content of a file into a buffer. The caller is responsible for freeing the buffer.
 * The file is read in binary mode to avoid issues with line endings on different platforms. The buffer is null-terminated.
 * 
 * @param filename The path to the file to read.
 * @param out_buf Pointer to a char* that will be set to point to the allocated buffer containing the file contents.
 * @param out_size Pointer to a size_t that will be set to the size of the file contents (excluding null terminator).
 * @return 0=success, 1=error (check errno for details)
 */
int read_file(const char* filename, char** out_buf, size_t* out_size) {

    if (check_toml_file(filename) != 0) {
        fprintf(stderr, "File: %s is not a .toml file.\n", filename);
        return EXIT_FAILURE;
    }

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
    return EXIT_SUCCESS;
}