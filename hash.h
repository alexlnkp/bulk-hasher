#ifndef HASH_H
#define HASH_H

typedef struct HashingDirectory {
    size_t num_files;
    char** files;
} HashingDirectory;

typedef struct StackNode {
    char* path;
    struct StackNode* next;
} StackNode;

void convert_hash_to_str(unsigned char* hash, char* hash_str);

void hash_file(FILE *fp, sha256_ctx *ctx);
char** hash_files(HashingDirectory* dir);

HashingDirectory* get_filenames(char* root_path);

void regenerate_hashes(char* path);

size_t Ccheck_hashes_against_file(const char* hash_list_filename);

static PyObject* check_hashes_against_file(PyObject* self, PyObject* args);
static PyObject* version(PyObject* self);

static PyMethodDef HashMethods[] = {
    {"check_hashes_against_file", (PyCFunction)check_hashes_against_file, METH_VARARGS, "Check all files in the file specified against corresponding SHA256 hashes, returns the number of mismatched hashes"},
    {"version", (PyCFunction)version, METH_NOARGS, "Get the version of the program"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef hashmodule = {
    PyModuleDef_HEAD_INIT,
    "hash",
    "SHA256 hashing module",
    -1,
    HashMethods
};

PyMODINIT_FUNC PyInit_hasher() {
    return PyModule_Create(&hashmodule);
}

#endif // HASH_H