#include "../worker.h"

#include "../server.h"
#include "../task_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <netinet/ip.h>

// TODO: Handle errors
WorkerArr* worker_create(int count, Server* server) {
    WorkerArr* arr = malloc(sizeof(WorkerArr));
    if (arr == NULL)
    {
        fprintf(stderr, "ERROR::Couldn't allocate memory for worker arr\n");
        return NULL;
    }
    
    
    pthread_t* threads = malloc( sizeof(pthread_t) * count);
    if (threads == NULL)
    {
        fprintf(stderr, "ERROR::Couldn't allocate memory for workers\n");
        return NULL;
    }
    
    int created_worker_count = 0;
    for (int i = 0; i < count; i++)
    {
        int ret = pthread_create(threads + created_worker_count, NULL, worker_routine, (void*)server);
        if (ret != 0)
        {
            fprintf(stderr, "Couldn't create a worker thread number %i", i);
            continue;
        }
        
        created_worker_count++;
    }
    
    arr->worker_threads = threads;
    arr->worker_count = created_worker_count;

    return arr;
}

void worker_destroy(WorkerArr* workers) {
    for (int i = 0; i < workers->worker_count; i++) {
        pthread_join(workers->worker_threads[i], NULL);
    }

    free(workers->worker_threads);
    free(workers);
}

void* worker_routine(void* arg) {
    Server* server = (Server*)arg;

    while (server->server_running) {

        int connection_fd = task_queue_dequeue(server->task_queue);
  
        if (connection_fd == -1) {
            if (!server->server_running)
            {
                break;
            }
            continue;
        }

        printf("Client connected\n");
        // Dummy msg ... TODO: do http 
        char msg[] = "Hello from server";
        send(connection_fd, msg, sizeof(msg), 0);
        close(connection_fd);
    }

    return NULL;
}