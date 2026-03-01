#include "../server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <pthread.h>

#include <signal.h>

#define TASK_QUEUE_CAP 100

//// SERVER GLOBAL VARIABLE ... set when enter server_run()
Server* g_server = NULL;

static void interrupt_server(int) {
    g_server->server_running = false;
    shutdown(g_server->socket_fd, SHUT_RDWR);
}

Server* server_init(const char* ip_addr, uint16_t port, int worker_count) {
    int err = 0;
    Server* server = malloc(sizeof(Server));
    memset(server, 0, sizeof(*server));

    server->task_queue = task_queue_init(TASK_QUEUE_CAP);

    server->sock_addr.sin_family = AF_INET;
    server->sock_addr.sin_port = htons(port);
    err = inet_pton(AF_INET, ip_addr, &server->sock_addr.sin_addr);
    if (err == 0)
    {
        printf("ERROR::wrong ip address : %s\n", ip_addr);
        free(server);
        return NULL;
    }
        
    // Initializing host address
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd  == -1)
    {
        printf("ERROR::couldn't create socket\n");
        free(server);
        return NULL;
    }

    // I don't understand this but make running program easier
    int opt = 1;
    setsockopt(server->socket_fd , SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    err = bind(server->socket_fd , (struct sockaddr*)&server->sock_addr, sizeof(server->sock_addr));
    if (err == -1)
    {
        printf("ERROR::couldn't bind socket\n");
        close(server->socket_fd);
        free(server);
        return NULL;
    }
    
    err = listen(server->socket_fd , 1);
    if (err == -1)
    {
        printf("ERROR::listining error\n");
        close(server->socket_fd);
        free(server);
        return NULL;
    }

    server->workers = worker_create(worker_count, server);
    server->server_running = true;

    return server;
}

// Server is NULL after this
void server_close(Server* server) {
    task_queue_signal_destroy(server->task_queue);
    worker_destroy(server->workers);
    task_queue_destroy(server->task_queue);
    
    close(server->socket_fd);
    free(server);
}

// dummy function for app to run infinitly
void server_run(Server* server) {

    g_server = server;
    signal(SIGINT, interrupt_server);

    while (server->server_running)
    {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        int connection_fd = accept(server->socket_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&peer_addr_len);
        
        if (connection_fd == -1) {
            if (!server->server_running)
            {
                break;
            }
            fprintf(stderr ,"ERROR::Accepting connection error");
            continue;
        }

        task_queue_enqueue(server->task_queue, connection_fd);
    }
}
