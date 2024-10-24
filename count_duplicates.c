#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define HASH_SIZE SHA256_DIGEST_LENGTH
#define HASH_TABLE_SIZE 65536

typedef struct HashEntry {
    unsigned char hash[HASH_SIZE];
    int count;
    struct HashEntry *next;
} HashEntry;

HashEntry *hash_table[HASH_TABLE_SIZE] = {NULL};

uint32_t hash_key(const unsigned char *hash) {
    return (hash[0] << 8 | hash[1]) % HASH_TABLE_SIZE;
}

void insert_or_update(const unsigned char *hash) {
    uint32_t key = hash_key(hash);
    HashEntry *entry = hash_table[key];

    while (entry) {
        if (memcmp(entry->hash, hash, HASH_SIZE) == 0) {
            entry->count++;
            return;
        }
        entry = entry->next;
    }

    entry = malloc(sizeof(HashEntry));
    memcpy(entry->hash, hash, HASH_SIZE);
    entry->count = 1;
    entry->next = hash_table[key];
    hash_table[key] = entry;
}

void free_hash_table() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = hash_table[i];
        while (entry) {
            HashEntry *temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
}

void calculate_sha256(const unsigned char *data, unsigned char *hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, PAGE_SIZE);
    SHA256_Final(hash, &sha256);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    unsigned char buffer[PAGE_SIZE];
    unsigned char hash[HASH_SIZE];
    size_t read_size;

    while ((read_size = fread(buffer, 1, PAGE_SIZE, file)) == PAGE_SIZE) {
        calculate_sha256(buffer, hash);
        insert_or_update(hash);
    }

    fclose(file);

    int total_duplicates = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = hash_table[i];
        while (entry) {
            if (entry->count > 1) {
                total_duplicates += entry->count;
            }
            entry = entry->next;
        }
    }

    printf("Total number of duplicate occurrences: %d\n", total_duplicates);
    free_hash_table();

    return EXIT_SUCCESS;
}

