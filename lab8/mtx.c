#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define READ_THREADS_NUM 10
#define WRITE_LIMIT 10

typedef struct {
    int array[100];
    int next_write;
    pthread_mutex_t mutex;
} thread_array;

void* write_thread(void* thread_data) {
    thread_array* shared_array = (thread_array*)thread_data;
    for (int i = 0; i < WRITE_LIMIT; i++) {
        usleep(10000);
        pthread_mutex_lock(&shared_array->mutex);

        shared_array->array[shared_array->next_write] = shared_array->next_write;
        printf("Written: %d\n", shared_array->array[shared_array->next_write]);

        shared_array->next_write++;
        usleep(200000);
        pthread_mutex_unlock(&shared_array->mutex);
    }

    return NULL;
}

void* read_thread(void* thread_data) {
    thread_array* shared_array = (thread_array*)thread_data;
    usleep(100000);
    for (int i = 0; i < WRITE_LIMIT; i++) {
        usleep(10000);
        pthread_mutex_lock(&shared_array->mutex);
        if (i < shared_array->next_write) {
            printf("Read: array[%d] = %d tid: %lx\n", i, shared_array->array[i], pthread_self());
        }
        
        pthread_mutex_unlock(&shared_array->mutex);
        usleep(100000);
    }

    return NULL;
}

int main() {
    thread_array shared_array;
    shared_array.next_write = 0;

    pthread_mutex_init(&shared_array.mutex, NULL);
    pthread_t writing_array;
    pthread_t reading_threads[READ_THREADS_NUM];
    pthread_create(&writing_array, NULL, write_thread, (void*)&shared_array);
    
    for (int i = 0; i < READ_THREADS_NUM; i++) {
        pthread_create(&reading_threads[i], NULL, read_thread, (void*)&shared_array);
    }
    pthread_join(writing_array, NULL);
    
    for (int i = 0; i < READ_THREADS_NUM; i++) {
        pthread_join(reading_threads[i], NULL);
    }
    pthread_mutex_destroy(&shared_array.mutex);
    return 0;
}
