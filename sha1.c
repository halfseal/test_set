#include <openssl/evp.h>
#include <openssl/sha.h>  // For SHA_DIGEST_LENGTH
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PAGE_SIZE 4096  // 4KB page size
#define NUM_ITERATIONS 10  // Number of iterations for averaging

// Function to calculate SHA-1 hash using EVP interface
void calculate_sha1(const unsigned char *data, unsigned char *hash, unsigned int *hash_len) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();  // Create a new context
    EVP_DigestInit_ex(ctx, EVP_sha1(), NULL);  // Initialize for SHA-1
    EVP_DigestUpdate(ctx, data, PAGE_SIZE);  // Add data to hash
    EVP_DigestFinal_ex(ctx, hash, hash_len);  // Finalize the hash
    EVP_MD_CTX_free(ctx);  // Free the context
}

// Function to get the current time in seconds (with nanoseconds precision)
double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

double iteration_time = 0.0;
// Function to read 4KB pages from a file and calculate their SHA-1 hash
void process_file_sha1(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    unsigned char buffer[PAGE_SIZE];
    unsigned char hash[SHA_DIGEST_LENGTH];  // Use OpenSSL's SHA_DIGEST_LENGTH
    unsigned int hash_len;
    size_t read_size;
    int page_count = 0;

    // Read the file 4KB at a time and calculate SHA-1 for each page
    while ((read_size = fread(buffer, 1, PAGE_SIZE, file)) == PAGE_SIZE) {
        double start_time = get_time_in_seconds();
        calculate_sha1(buffer, hash, &hash_len);  // Calculate SHA-1 hash
        double end_time = get_time_in_seconds();
        iteration_time += (end_time - start_time);

        page_count++;
    }

    fclose(file);
}

// Main function to measure the average time for 10 iterations
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    double total_time = 0.0;

    // Run the process 10 times and accumulate the total time
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        iteration_time = 0.0;
        process_file_sha1(argv[1]);  // Process the file
        printf("Iteration %d: %.6f seconds\n", i + 1, iteration_time);

        total_time += iteration_time;
    }

    // Calculate and print the average time
    double average_time = total_time / NUM_ITERATIONS;
    printf("Average time for %d iterations: %.6f seconds\n", NUM_ITERATIONS, average_time);

    return EXIT_SUCCESS;
}
