#pragma once

#include <pthread.h>

typedef struct WorkerArr {
    pthread_t* worker_threads;
    int worker_count;
}WorkerArr;

typedef struct Server Server;

void* worker_routine(void* arg);
WorkerArr* worker_create(int count, Server* server);
void worker_destroy(WorkerArr* workers);