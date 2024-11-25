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
#include <sys/wait.h>

int shmid = 0;
char* shm_name = "shared_memory";
char* addr = NULL;

void sendError(const char* func, const char* message) {
    fprintf(stderr, "Error in %s: %s\n", func, message);
    exit(EXIT_FAILURE);
}

void handler(int sig) {
    printf("[SIGNAL HANDLER] Signal %d received\n", sig);
    
    if (addr != NULL && shmdt(addr) < 0) {
        int err = errno;
        fprintf(stderr, "In shmdt %s (%d)\n", strerror(err), err);
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        int err = errno;
        fprintf(stderr, "In shmctl %s (%d)\n", strerror(err), err);
        exit(1);
    }

    unlink(shm_name);
    exit(0);
}

int main(int argc, char** argv) {
    (void)argc, (void)argv;
    int fd = open(shm_name, O_CREAT|O_EXCL, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        fprintf(stderr, "The process is running\n");
	close(fd);
        return 1;
    }

    close(fd);

    if (signal(SIGINT, handler) == SIG_ERR || signal(SIGTERM, handler) == SIG_ERR) {
        perror("Error in signal");
        return 1;
    }

    key_t key = ftok(shm_name, 1);
    if (key == -1) {
        sendError("ftok", strerror(errno));
    }

    shmid = shmget(key, 1024, 0666 | IPC_CREAT | IPC_EXCL);
    if (shmid == -1) {
        sendError("shmget", strerror(errno));
    }

    addr = (char*)shmat(shmid, NULL, 0);
    if (addr == (char*)-1) {
        sendError("shmat", strerror(errno));
    }

    while (1) {
        time_t mytime = time(NULL);
        struct tm* now = localtime(&mytime);
        char str[100];
        snprintf(str, sizeof(str), "[SEND] Time: %02d:%02d:%02d, my pid: %d\n", 
                 now->tm_hour, now->tm_min, now->tm_sec, getpid());
        strcpy(addr, str);
        sleep(3);
    }

    return 0;
}
