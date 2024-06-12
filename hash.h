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

static PyMethodDef HashMethods[] = {
    {"hash_file", (PyCFunction)hash_file, METH_VARARGS, "Get the SHA256 hash of the file specified"},
    {"check_hashes_against_file", (PyCFunction)check_hashes_against_file, METH_VARARGS, "Check all files in the file specified against corresponding SHA256 hashes, returns the number of mismatched hashes"},
    {"regenerate_hashes", (PyCFunction)regenerate_hashes, METH_VARARGS, "Regenerate SHA256 hashes for all files in the directory specified, writing the results to the specified file"},
    {"get_hash_from_file", (PyCFunction)get_hash_from_file, METH_VARARGS, "Get the SHA256 hash of the file specified in the sha256 file"},
    {"version", (PyCFunction)version, METH_NOARGS, "Get the version of the program"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef bulkhashermodule = {
    PyModuleDef_HEAD_INIT,
    "hash",
    "SHA256 hashing module",
    -1,
    HashMethods,
    .m_slots = NULL,
};

PyMODINIT_FUNC PyInit_bulkhasher() {
    return PyModule_Create(&bulkhashermodule);
}

#endif // HASH_H