#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/sem.h>

char* shm_name = "shared_memory";
int size = 1024;
int shmid = 0;
char* addr = NULL;
int semid = 0;

struct sembuf sem_lock = {0, -1, 0}, sem_open = {0, 1, 0};

void sendError(const char* func, const char* message) {
    fprintf(stderr, "Error in %s: %s\n", func, message);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    (void)argc, (void)argv;

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

    semid = semget(key, 1, 0666);
    if (semid == -1) {
        sendError("semget", strerror(errno));
    }
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        sendError("semctl", strerror(errno));
    }

    while (1) {
        if (semop(semid, &sem_lock, 1) == -1) {
            sendError("semop wait", strerror(errno));
        }

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

        if (semop(semid, &sem_open, 1) == -1) {
            sendError("semop signal", strerror(errno));
        }
        sleep(1);
    }

    if (addr != NULL) {
        if (shmdt(addr) < 0) {
            sendError("shmdt", strerror(errno));
        }
    }
    return 0;
}
