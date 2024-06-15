import bulkhasher

print(bulkhasher.version())

print(bulkhasher.hash_file("build/bulkhasher.so"))

bulkhasher.regenerate_hashes("assets", "SHA256")