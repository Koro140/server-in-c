#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct Server {
    int socket_fd;
    struct sockaddr_in sock_addr;
    
    pthread_t* worker_threads;
    int worker_count;

    volatile bool server_running;
}Server;

Server* server_init(const char* ip_addr, uint16_t port, int worker_count);
void server_close(Server* server);
void* worker_routine(void* arg);
pthread_t* worker_create(int count, Server* server);

Server* server_init(const char* ip_addr, uint16_t port, int worker_count) {
    int err = 0;
    Server* server = malloc(sizeof(Server));
    memset(server, 0, sizeof(*server));

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

    server->worker_threads = worker_create(worker_count, server);
    server->worker_count = worker_count;
    server->server_running = true;

    return server;
}

// Server is NULL after this
void server_close(Server* server) {
    shutdown(server->socket_fd, SHUT_RDWR);
    for (int i = 0; i < server->worker_count; i++)
    {
        pthread_join(server->worker_threads[i], NULL);
    }
    
    close(server->socket_fd);

    free(server->worker_threads);
    free(server);
}

void* worker_routine(void* arg) {
    Server* server = (Server*)arg;
 
    while (server->server_running) {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);

        int connection_fd = accept(server->socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_len);

        if (connection_fd == -1) {
            if (!server->server_running)
            {
                break;
            }
            fprintf(stderr ,"ERROR::Accepting connection error");
            continue;
        }
        else {
            printf("Client connected\n");

            // Dummy msg ... TODO: do http 
            char msg[] = "Hello from server";
            send(connection_fd, msg, sizeof(msg), 0);

            close(connection_fd);
        }
    }

    return NULL;
}

// TODO: Handle errors
pthread_t* worker_create(int count, Server* server) {
    pthread_t* threads = malloc( sizeof(pthread_t) * count);

    for (int i = 0; i < count; i++)
    {
        pthread_create(threads + i, NULL, worker_routine, (void*)server);
    }
    
    return threads;
}

// dummy function for app to run infinitly
void server_run(Server* server) {
    while (server->server_running)
    {
        if (getchar() == 'c') {
            server->server_running = false;
        }
    }
}

int main(void) {
    Server* server = server_init("127.0.0.1", 6969, 5);
    if (server == NULL)
    {
        fprintf(stderr ,"Couldn't initialize server\n");
        return 1;
    }
    
    server_run(server);

    server_close(server);
    return 0;
}