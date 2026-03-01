#pragma once

#include <netinet/ip.h>
#include <stdbool.h>

typedef struct WorkerArr WorkerArr;
typedef struct TaskQueue TaskQueue;

typedef struct Server {
    int socket_fd;
    struct sockaddr_in sock_addr;
    
    WorkerArr* workers;
    TaskQueue* task_queue;

    volatile bool server_running;
}Server;

Server* server_init(const char* ip_addr, uint16_t port, int worker_count, int task_queue_size);
void server_run(Server* server);
void server_close(Server* server);