#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

void func() {
    printf("[atexit] I'm atexit for process %d\n", getpid());
}

void handler(int sig) {
    switch(sig) {
        case SIGTERM:
            printf("[signal] Signal 15 received, my pid is %d\n", getpid());
            break;
        case SIGINT:
            printf("[signal] Signal 2 received, my pid is %d\n", getpid());
            break;
        default:
            printf("[signal] Signal %d received, my pid is %d\n", sig, getpid());
            break;
    }
}

int main(int argc, char** argv) {

    (void)argc; (void)argv;

    if (signal(SIGINT, handler) == SIG_ERR) {
	perror("Error signal\n");
    }
  
    struct sigaction sigAction;
    sigAction.sa_handler = handler;
    sigAction.sa_flags = SA_SIGINFO; 
    if (sigaction(SIGTERM, &sigAction, NULL) == -1) {
	perror("Error sigaction\n");
    }

    if (atexit(func) != 0) {
        perror("Atexit unsuccess\n");
    }

    pid_t res = 0;

    switch (res = fork()) {
        case -1: {
            int err = errno;
            fprintf(stderr, "Fork error: %s (%d)\n", strerror(err), err);
            break;
        }
        case 0: {
            // В дочернем процессе
            printf("[CHILD] I'm child of %d, my pid is %d\n", getppid(), getpid());
            break;
        }
        default: {
            // В родительском процессе
            int cres;
	    sleep(5);		
            wait(&cres);
            printf("[PARENT] I'm parent of %d, my pid is %d, my parent pid is %d\n", res, getpid(), getppid());
            printf("[PARENT] Child exit code %d\n", WEXITSTATUS(cres));
            break;
        }
    }
    return 0;
}
