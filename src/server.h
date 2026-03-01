#pragma once

#include "worker.h"
#include "task_queue.h"

#include <netinet/ip.h>

typedef struct Server {
    int socket_fd;
    struct sockaddr_in sock_addr;
    
    WorkerArr* workers;
    TaskQueue* task_queue;

    volatile bool server_running;
}Server;

Server* server_init(const char* ip_addr, uint16_t port, int worker_count);
void server_run(Server* server);
void server_close(Server* server);