#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <megabytes> [seconds]\n", argv[0]);
        return 1;
    }

    int megabytes = atoi(argv[1]);
    int seconds = (argc == 3) ? atoi(argv[2]) : -1;

    size_t bytes = megabytes * 1024 * 1024;
    char *memory = (char *)malloc(bytes);

    if (memory == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return 1;
    }

    fprintf(stdout, "Allocated %d MB of memory.\n", megabytes);

    while (1) {
        for (size_t i = 0; i < bytes; i += 4096) {
            memory[i] = i % 256; 
        }

        if (seconds > 0) {
            sleep(1);
            seconds--;
            if (seconds == 0)
                break;
        }
    }

    free(memory);
    fprintf(stdout, "Memory released.\n");

    return 0;
}

