#include <Python.h>
#include <omp.h>
#include "../sha2.h"

#include "../hash.h"

int main(void) {
    char* hs = C_get_hash_from_file("assets/hubert/hubert_base.pt", "SHA256");
    printf("%s\n", hs);

    FILE* fp = fopen("assets/hubert/hubert_base.pt", "r");
    sha256_ctx ctx;
    sha256_init(&ctx);
    C_hash_file(fp, &ctx);

    fclose(fp);
    char hash_str[SHA256_DIGEST_SIZE * 2 + 1];
    convert_hash_to_str(ctx.block, hash_str);

    printf("%s\n", hash_str);

    C_regenerate_hashes("assets", "SHA256");
    return 0;
}