#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NUM_THREADS 9
#define NUM_BLOCKS 3

void* allocate_memory(void* arg) {
    size_t block_sizes[NUM_BLOCKS] = {16, 1024, 1048576}; // Block sizes: 16 bytes, 1 KB, and 1 MB

    for (int i = 0; i < NUM_BLOCKS; i++) {
        size_t block_size = block_sizes[i];
        void* addr = malloc(block_size);
        printf("Thread %ld allocated %ld bytes at address: %p\n", pthread_self(), block_size, addr);
    }

    pthread_exit(NULL);
}

void* fill_memory(void* arg) {
    int thread_num = *(int*)arg;
    size_t block_sizes[NUM_BLOCKS] = {16, 1024, 1048576};

    for (int i = 0; i < NUM_BLOCKS; i++) {
        size_t block_size = block_sizes[i];
        void* addr = malloc(block_size);
        memset(addr, thread_num, block_size);
    }

    pthread_exit(NULL);
}

void* save_memory(void* arg) {
    int thread_num = *(int*)arg;
    size_t block_sizes[NUM_BLOCKS] = {16, 1024, 1048576};

    for (int i = 0; i < NUM_BLOCKS; i++) {
        size_t block_size = block_sizes[i];
        void* addr = malloc(block_size);
        char file_name[20];
        sprintf(file_name, "memory_block_%d.bin", thread_num);
        int fd = open(file_name, O_RDWR | O_CREAT, 0666);
        ftruncate(fd, block_size);
        void* mem_mapped = mmap(NULL, block_size, PROT_WRITE, MAP_SHARED, fd, 0);
        memcpy(mem_mapped, addr, block_size);
        munmap(mem_mapped, block_size);
        close(fd);
        free(addr);
    }

    pthread_exit(NULL);
}

int main() {
    int M = 5; // Number of repetitions

    for (int m = 0; m < M; m++) {
        pthread_t threads[NUM_THREADS];
        int thread_args[NUM_THREADS];

        // Create and run threads for memory allocation
        for (int i = 0; i < NUM_THREADS / 3; i++) {
            pthread_create(&threads[i], NULL, allocate_memory, NULL);
        }

        // Create and run threads for memory filling
        for (int i = NUM_THREADS / 3; i < (2 * NUM_THREADS) / 3; i++) {
            thread_args[i] = i % (NUM_THREADS / 3);
            pthread_create(&threads[i], NULL, fill_memory, &thread_args[i]);
        }

        // Create and run threads for memory saving
        for (int i = (2 * NUM_THREADS) / 3; i < NUM_THREADS; i++) {
            thread_args[i] = i % (NUM_THREADS / 3);
            pthread_create(&threads[i], NULL, save_memory, &thread_args[i]);
        }

        // Wait for all threads to finish
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        printf("Iteration %d completed\n", m + 1);
    }

    return 0;
}

