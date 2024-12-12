#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define ARRAY_SIZE 10
#define NUM_READERS 10

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int shared_array[ARRAY_SIZE];
int write_count = 0;

void* writer_thread(void* arg) {
    while (write_count < ARRAY_SIZE) {
        pthread_rwlock_wrlock(&rwlock);

        shared_array[write_count] = write_count;
        printf("Writer updated array at index: %d\n", shared_array[write_count]);
        write_count++;
	usleep(100000);
        		
        pthread_rwlock_unlock(&rwlock);
        usleep(100000);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    int tid = *((int*)arg);
    while(1) {
        pthread_rwlock_rdlock(&rwlock);

	if (write_count >= ARRAY_SIZE) {
            pthread_rwlock_unlock(&rwlock);
            break;
	}
        printf("Reader %d reads array: ", tid);
        for (int i = 0; i < write_count; i++) {
            printf("%d ", shared_array[i]);
        }
        printf(", tid: [%lx]\n", pthread_self());
        usleep(100000);

        pthread_rwlock_unlock(&rwlock);
        usleep(100000);
    }
    pthread_rwlock_rdlock(&rwlock);

    printf("Reader %d reads array: ", tid);
    for (int i = 0; i < write_count; i++) {
        printf("%d ", shared_array[i]);
    }
    printf(", tid: [%lx]\n", pthread_self());
    pthread_rwlock_unlock(&rwlock);

    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];
    pthread_t writer;
    int reader_ids[NUM_READERS];

    pthread_rwlock_init(&rwlock, NULL);

    pthread_create(&writer, NULL, writer_thread, NULL);

    for (int i = 0; i < NUM_READERS; ++i) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }

    for (int i = 0; i < NUM_READERS; ++i) {
        void* res = NULL;
        int join_res = pthread_join(readers[i], &res);
        if (join_res != 0) {
            int err = errno;
            printf("ERROR IN JOIN (reader %d): %s(%d)\n", i, strerror(err), err);
        }
    }

    void* res = NULL;
    int join_res = pthread_join(writer, &res);
    if (join_res != 0) {
        printf("ERROR IN JOIN (writer): %s\n", strerror(errno));
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
