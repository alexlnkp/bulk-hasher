#include <Python.h>
#include <omp.h>
#include <dirent.h>
#include <stdbool.h>
#include "sha2.h"

#include "hash.h"

#define BUFFER_SIZE  16384 // 16 KiB buffer for reading files
#define FILES_TO_STORE 256 // Maximum number of files to store in memory
#define READ_BUFFER   4096 // Read at most 4 KiB per line

#define PARALLEL_PROCESSES 16
#define PARALLEL_WORKERS 8

#define CHECK_ALLOC(x, y) \
    if (x == NULL) { PyErr_NoMemory(); return y; }

omp_lock_t lock;

int strend(const char *s, const char *t) {
    size_t ls = strlen(s); // find length of s
    size_t lt = strlen(t); // find length of t
    if (ls >= lt) { // check if t can fit in s
        // point s to where t should start and compare the strings from there
        return (0 == memcmp(t, s + (ls - lt), lt));
    }
    return 0; // t was longer than s
}

// Hash a file using SHA256
void hash_file(FILE *fp, sha256_ctx *ctx) {
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
        sha256_update(ctx, buffer, bytes_read);
    sha256_final(ctx, ctx->block);
}

void convert_hash_to_str(unsigned char* hash, char* hash_str) {
    // Convert each byte of the hash to a 2-character hexadecimal string
    for (int i = 0; i < SHA256_DIGEST_SIZE; ++i)
        sprintf(&hash_str[i * 2], "%02x", hash[i]);
    
    hash_str[SHA256_DIGEST_SIZE * 2] = '\0';
}

char** hash_files(HashingDirectory* dir) {
    omp_set_num_threads(PARALLEL_PROCESSES);

    char** hashes = calloc(dir->num_files, sizeof(char*));
    CHECK_ALLOC(hashes, NULL);

    for (size_t i = 0; i < dir->num_files; ++i) {
        hashes[i] = malloc(SHA256_DIGEST_SIZE * 2 + 1);
        CHECK_ALLOC(hashes[i], NULL);
    }

    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < dir->num_files; ++i) {
        sha256_ctx ctx;
        sha256_init(&ctx);
        FILE* fp = fopen(dir->files[i], "r");
        if (fp == NULL) {
            omp_set_lock(&lock);
                PyErr_SetFromErrno(PyExc_OSError);
                printf("Error opening file: %s\n", dir->files[i]);
            omp_unset_lock(&lock);
            free(hashes[i]); hashes[i] = NULL;
            continue;
        }

        hash_file(fp, &ctx);
        convert_hash_to_str(ctx.block, hashes[i]);

        fclose(fp);
    }

    omp_destroy_lock(&lock);
    return hashes;
}

// Utility functions for stack operations
void push(StackNode** top, const char* path) {
    StackNode* new_node = malloc(sizeof(StackNode));
    CHECK_ALLOC(new_node, ;);
    new_node->path = strdup(path);
    CHECK_ALLOC(new_node->path, ;);
    new_node->next = *top;
    *top = new_node;
}

char* pop(StackNode** top) {
    if (*top == NULL) return NULL;
    StackNode* temp = *top;
    char* path = temp->path;
    *top = temp->next;
    free(temp);
    return path;
}

void free_stack(StackNode** top) {
    while (*top != NULL) {
        char* path = pop(top);
        free(path);
    }
}

HashingDirectory* get_filenames(char* root_path) {
    HashingDirectory* directories = malloc(sizeof(HashingDirectory));
    if (!directories) return NULL;
    directories->num_files = 0;
    size_t capacity = FILES_TO_STORE;
    directories->files = malloc(sizeof(char*) * capacity);
    if (!directories->files) {
        free(directories);
        return NULL;
    }

    StackNode* stack = NULL;
    push(&stack, strdup(root_path)); // Duplicate the root_path to be able to free it later

    while (stack != NULL) {
        char* path = pop(&stack);

        DIR* d = opendir(path);
        if (d == NULL) {
            fprintf(stderr, "Error opening directory: %s\n", strerror(errno));
            free(path);
            continue;
        }

        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                // Check if we need to resize the array
                if (directories->num_files >= capacity) {
                    capacity *= 2; // Double the capacity
                    char** resized_array = realloc(directories->files, sizeof(char*) * capacity);
                    if (!resized_array) {
                        // Handle error: free all resources
                        closedir(d);
                        free(path);
                        for (size_t i = 0; i < directories->num_files; ++i) {
                            free(directories->files[i]);
                        }
                        free(directories->files);
                        free(directories);
                        free_stack(&stack);
                        return NULL;
                    }
                    directories->files = resized_array;
                }

                // Allocate and store the filename
                directories->files[directories->num_files] = malloc(strlen(path) + strlen(dir->d_name) + 2);
                if (!directories->files[directories->num_files]) {
                    // Handle error: free all resources
                    closedir(d);
                    free(path);
                    for (size_t i = 0; i < directories->num_files; ++i) {
                        free(directories->files[i]);
                    }
                    free(directories->files);
                    free(directories);
                    free_stack(&stack);
                    return NULL;
                }
                sprintf(directories->files[directories->num_files], "%s/%s", path, dir->d_name);
                directories->num_files++;
            } else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                // Push sub-directory to the stack
                char* sub_path = malloc(strlen(path) + strlen(dir->d_name) + 2);
                if (!sub_path) {
                    // Handle error: free all resources
                    closedir(d);
                    free(path);
                    for (size_t i = 0; i < directories->num_files; ++i) {
                        free(directories->files[i]);
                    }
                    free(directories->files);
                    free(directories);
                    free_stack(&stack);
                    return NULL;
                }
                sprintf(sub_path, "%s/%s", path, dir->d_name);
                push(&stack, sub_path);
            }
        }

        closedir(d);
        free(path);
    }

    free_stack(&stack);
    return directories;
}

void regenerate_hashes(char* path) {
    omp_set_num_threads(PARALLEL_PROCESSES);

    HashingDirectory* files = get_filenames(path);
    char** hashes = hash_files(files);

    FILE* fp = fopen("SHA256", "w");
    if (fp == NULL) {
        PyErr_SetFromErrno(PyExc_OSError);
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return;
    }

    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < files->num_files; ++i) {
        if (!(strend(files->files[i], ".gitignore")) && !(strend(files->files[i], ".git")) && strstr(files->files[i], "/weights/") == NULL)
            fprintf(fp, "%s = %s\n", files->files[i], hashes[i]);
    }

    for (size_t i = 0; i < files->num_files; ++i) {
        free(files->files[i]);
        free(hashes[i]);
    }

    free(files);
    free(hashes);
}

size_t process_line_of_SHA256_file(char* line) {
    sha256_ctx ctx;
    sha256_init(&ctx);

    size_t mismatched_hashes = 0;
    
    // Remove newline character, if present
    char* newline = strchr(line, '\n');
    if (newline) *newline = '\0';

    // Parse the line to extract filename and stored hash
    char* filename = strtok(line, " = ");
    char* stored_hash = strtok(NULL, " = ");

    if (filename && stored_hash) {
        FILE* fp = fopen(filename, "r");
        if (fp == NULL) { PyErr_SetFromErrno(PyExc_OSError); printf("Error opening file: %s\n", filename); return 0; }

        hash_file(fp, &ctx);

        if (ferror(fp)) {
            printf("Error reading file: %s\n", filename);
            fclose(fp);
            return 0;
        }

        fclose(fp);

        char computed_hash[SHA256_DIGEST_SIZE * 2 + 1];
        convert_hash_to_str(ctx.block, computed_hash);

        // Compare the computed hash with the stored hash
        if ((strcmp(computed_hash, stored_hash) != 0)) {
            mismatched_hashes++;
            printf("Hash mismatch: %s\n", filename);
        }
    }

    return mismatched_hashes;
}

size_t Ccheck_hashes_against_file(const char* hash_list_filename) {
    size_t mismatched_hashes = 0;

    FILE* file = fopen(hash_list_filename, "r");
    if (file == NULL) { PyErr_SetFromErrno(PyExc_OSError); return -1; }

    char line[READ_BUFFER];
    while (fgets(line, READ_BUFFER, file) != NULL)
        mismatched_hashes += process_line_of_SHA256_file(line);

    fclose(file);

    return mismatched_hashes;
}

static PyObject* check_hashes_against_file(PyObject* self, PyObject* args) {
    const char* hash_list_filename;
    if (!PyArg_ParseTuple(args, "s", &hash_list_filename)) return NULL;
    return PyLong_FromSize_t(Ccheck_hashes_against_file(hash_list_filename));

}

static PyObject* version(PyObject* self) {
    return Py_BuildValue("s", "0.0.1");
}

int main(void) {
    size_t mismatched_hashes = Ccheck_hashes_against_file("SHA256");
    printf("Number of mismatched hashes: %zu\n", mismatched_hashes);

    // regenerate_hashes("assets");
    return 0;
}
