#ifndef HASH_H
#define HASH_H

#define BUFFER_SIZE  16384 // 16 KiB buffer for reading files
#define FILES_TO_STORE 256 // Maximum number of files to store in memory
#define READ_BUFFER   4096 // Read at most 4 KiB per line

#define PARALLEL_PROCESSES 16

typedef struct HashingDirectory {
    size_t num_files;
    char** files;
} HashingDirectory;

typedef struct StackNode {
    char* path;
    struct StackNode* next;
} StackNode;

void convert_hash_to_str(unsigned char* hash, char* hash_str);

void C_hash_file(FILE *fp, sha256_ctx *ctx);
char** hash_files(HashingDirectory* dir);

HashingDirectory* get_filenames(char* root_path);

void C_regenerate_hashes(char* path, char* out_file);
size_t C_check_hashes_against_file(const char* hash_list_filename);
char* C_get_hash_from_file(char* file_to_hash, char* sha_file);

static PyObject* hash_file(PyObject* self, PyObject* args);
static PyObject* check_hashes_against_file(PyObject* self, PyObject* args);
static PyObject* regenerate_hashes(PyObject* self, PyObject* args);
static PyObject* get_hash_from_file(PyObject* self, PyObject* args);
static PyObject* version(PyObject* self);

#endif // HASH_H