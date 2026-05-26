#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_BLOCKS 50
#define BLOCK_SIZE (1024 * 1024) // 1 MiB
// 2^20      = 1048576
// 1024*1024 = 1048576

int main() {
    void *blocks[NUM_BLOCKS];
    int i;

    printf("Allocate %d blocks of 1 MiB\n", NUM_BLOCKS);

    for (i = 0; i < NUM_BLOCKS; i++) {
        blocks[i] = malloc(BLOCK_SIZE);

        if (blocks[i] == NULL) {
            fprintf(stderr, "Error: Allocation not possible for block %d (Limit reached!)\n", i);

            for (int j = 0; j < i; j++) {
                free(blocks[j]);
            }
            return 1;
        }

        memset(blocks[i], 0, BLOCK_SIZE); // Touch the memory to ensure it's actually allocated

        printf("Block %d allocated and initialized successfully.\n", i);

        usleep(100000); // 100ms
    }

    printf("Success: All blocks have been allocated.\n");

    // Release the memory at the end
    for (i = 0; i < NUM_BLOCKS; i++) {
        free(blocks[i]);
    }

    return 0;
}