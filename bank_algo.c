#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_P 10
#define MAX_R 10

int P, R;
int available[MAX_R];
int max[MAX_P][MAX_R];
int allocation[MAX_P][MAX_R];
int need[MAX_P][MAX_R];

pthread_mutex_t lock;

int total_granted = 0, total_denied = 0;

// Utility function to print current state
void print_state() {
    printf("\nCurrent State:\n");
    printf("Available: ");
    for (int i = 0; i < R; i++)
        printf("%d ", available[i]);
    printf("\n");

    printf("Allocation:\n");
    for (int i = 0; i < P; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < R; j++)
            printf("%d ", allocation[i][j]);
        printf("\n");
    }

    printf("Need:\n");
    for (int i = 0; i < P; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < R; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }
    printf("\n");
}

// Check if the system is in a safe state
bool is_safe() {
    int work[MAX_R];
    bool finish[MAX_P] = {false};

    for (int i = 0; i < R; i++)
        work[i] = available[i];

    int count = 0;
    while (count < P) {
        bool found = false;
        for (int p = 0; p < P; p++) {
            if (!finish[p]) {
                int j;
                for (j = 0; j < R; j++)
                    if (need[p][j] > work[j])
                        break;
                if (j == R) {
                    for (int k = 0; k < R; k++)
                        work[k] += allocation[p][k];
                    finish[p] = true;
                    found = true;
                    count++;
                }
            }
        }
        if (!found)
            return false;
    }
    return true;
}

// Try to request resources for a process
bool request_resources(int pid, int request[]) {
    printf("P%d requesting: ", pid);
    for (int i = 0; i < R; i++)
        printf("%d ", request[i]);
    printf("\n");

    // Check if request <= need
    for (int i = 0; i < R; i++) {
        if (request[i] > need[pid][i])
            return false;
    }
    // Check if request <= available
    for (int i = 0; i < R; i++) {
        if (request[i] > available[i])
            return false;
    }
    // Tentatively allocate
    for (int i = 0; i < R; i++) {
        available[i] -= request[i];
        allocation[pid][i] += request[i];
        need[pid][i] -= request[i];
    }

    if (is_safe()) {
        total_granted++;
        printf("Request granted to P%d.\n", pid);
        return true;
    } else {
        // Rollback
        for (int i = 0; i < R; i++) {
            available[i] += request[i];
            allocation[pid][i] -= request[i];
            need[pid][i] += request[i];
        }
        total_denied++;
        printf("Request denied to P%d (unsafe).\n", pid);
        return false;
    }
}

// Release all resources when process finishes
void release_resources(int pid) {
    printf("P%d has finished. Releasing resources.\n", pid);
    for (int i = 0; i < R; i++) {
        available[i] += allocation[pid][i];
        allocation[pid][i] = 0;
        need[pid][i] = 0;
    }
}

// Thread function for each process
void* process_code(void* param) {
    int pid = *(int*)param;
    free(param);

    while (true) {
        pthread_mutex_lock(&lock);

        bool done = true;
        for (int i = 0; i < R; i++) {
            if (need[pid][i] > 0) {
                done = false;
                break;
            }
        }
        if (done) {
            release_resources(pid);
            pthread_mutex_unlock(&lock);
            break;
        }

        int request[MAX_R];
        for (int i = 0; i < R; i++) {
            request[i] = rand() % (need[pid][i] + 1);
        }

        request_resources(pid, request);

        print_state();
        pthread_mutex_unlock(&lock);

        sleep(1); // Sleep to simulate time passing
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror("File open failed");
        return 1;
    }

    fscanf(fp, "%d %d", &P, &R);

    for (int i = 0; i < R; i++)
        fscanf(fp, "%d", &available[i]);

    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++)
            fscanf(fp, "%d", &max[i][j]);
        for (int j = 0; j < R; j++)
            fscanf(fp, "%d", &allocation[i][j]);
        for (int j = 0; j < R; j++)
            fscanf(fp, "%d", &need[i][j]);
    }

    fclose(fp);

    pthread_t processes[P];
    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < P; i++) {
        int* pid = malloc(sizeof(int));
        *pid = i;
        pthread_create(&processes[i], NULL, process_code, pid);
    }

    for (int i = 0; i < P; i++)
        pthread_join(processes[i], NULL);

    pthread_mutex_destroy(&lock);

    printf("\nAll processes completed.\n");
    printf("Granted requests: %d\n", total_granted);
    printf("Denied requests: %d\n", total_denied);

    return 0;
}
