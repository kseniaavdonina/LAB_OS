#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>

#define SEM_NAME "/my_semaphore"

char* shm_name = "shared_memory";
int size = 1024;
int shmid = 0;
char* addr = NULL;
sem_t *semaphore;

void sendError(const char* func, const char* message) {
    fprintf(stderr, "Error in %s: %s\n", func, message);
    exit(EXIT_FAILURE);
}

int main() {
    key_t key = ftok(shm_name, 1);
    if (key == (key_t)-1) {
        sendError("ftok", strerror(errno));
    }

    shmid = shmget(key, size, 0666);
    if (shmid < 0) {
        sendError("shmget", strerror(errno));
    }

    addr = shmat(shmid, NULL, 0);
    if (addr == (void*)-1) {
        sendError("shmat", strerror(errno));
    }

    semaphore = sem_open(SEM_NAME, 0);
    if (semaphore == SEM_FAILED) {
        perror("Failed to open semaphore");
        exit(1);
    }

    while (1) {
        sem_wait(semaphore);

        char str[100];
        strcpy(str, addr);
        time_t ttime = time(NULL);
        struct tm* m_time = localtime(&ttime);
        char timestr[30];
        strftime(timestr, sizeof(timestr), "%H:%M:%S", m_time);

        char output_str[200];
        snprintf(output_str, sizeof(output_str),
                 "[RECEIVE] Time: %s, my pid: %d, Received: %s\n",
                 timestr, getpid(), str);
        printf("%s", output_str);

        sem_post(semaphore);
        sleep(1);
    }

    if (addr != NULL) {
        if (shmdt(addr) < 0) {
            sendError("shmdt", strerror(errno));
        }
    }
    sem_close(semaphore);
    return 0;
}
