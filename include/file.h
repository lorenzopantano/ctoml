#ifndef CTOML_FILE_H
#define CTOML_FILE_H

#include <stddef.h>

/**
 * @brief Reads content of a file into a buffer. The caller is responsible for freeing the buffer.
 * The file is read in binary mode to avoid issues with line endings on different platforms. The buffer is null-terminated.
 * 
 * @param filename The path to the file to read.
 * @param out_buf Pointer to a char* that will be set to point to the allocated buffer containing the file contents.
 * @param out_size Pointer to a size_t that will be set to the size of the file contents (excluding null terminator).
 * @return 0=success, 1=error (check errno for details)
 */
int read_file(const char* filename, char** out_buf, size_t* out_size);

#endif // CTOML_FILE_H