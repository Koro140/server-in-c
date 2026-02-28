#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <signal.h>

#define TASK_QUEUE_CAP 100
#define WORKERS_COUNT 10

typedef struct TaskQueue {
    int* items;
    int capacity;

    int head;
    int tail;
    int size;

    pthread_mutex_t mtx;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

    bool queue_is_destroyed;
}TaskQueue;

typedef struct Server {
    int socket_fd;
    struct sockaddr_in sock_addr;
    
    pthread_t* worker_threads;
    int worker_count;

    TaskQueue* task_queue;
    volatile bool server_running;
    
}Server;

//// SERVER GLOBAL VARIABLE ... set when enter server_run()
Server* g_server = NULL;

Server* server_init(const char* ip_addr, uint16_t port, int worker_count);
void server_close(Server* server);

void* worker_routine(void* arg);
pthread_t* worker_create(int count, Server* server);
void worker_destroy(pthread_t* threads, int count);

TaskQueue* task_queue_init(int capacity);
void task_queue_enqueue(TaskQueue* queue, int item);
int task_queue_pop();
void task_queue_destroy(TaskQueue* queue);

bool task_queue_is_empty(TaskQueue *q) {
    return q->size == 0;
}

bool task_queue_is_full(TaskQueue *q) {
    return q->size == q->capacity;
}

TaskQueue* task_queue_init(int capacity) {
    TaskQueue* queue = malloc(sizeof(TaskQueue));
    if (queue == NULL)
    {
        fprintf(stderr, "Couldn't create a task queue\n");
        return NULL;
    }
    
    queue->items = malloc(capacity * sizeof(*queue->items));
    if (queue->items == NULL)
    {
        fprintf(stderr, "Couldn't create a memory for tasks in task queue\n");
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    queue->queue_is_destroyed = false;
    
    pthread_mutex_init(&queue->mtx, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    return queue;
}

void task_queue_enqueue(TaskQueue* queue, int item) {
    pthread_mutex_lock(&queue->mtx);
    while (task_queue_is_full(queue) && !queue->queue_is_destroyed)
    {
        pthread_cond_wait(&queue->not_full,&queue->mtx);
    }

    if (queue->queue_is_destroyed)
    {
        pthread_mutex_unlock(&queue->mtx);
        return;
    }

    queue->items[queue->tail] = item;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_mutex_unlock(&queue->mtx);
    return;
}

int task_queue_dequeue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mtx);
    while (task_queue_is_empty(queue) && !queue->queue_is_destroyed)
    {
        pthread_cond_wait(&queue->not_empty, &queue->mtx);
    }
    
    if (queue->queue_is_destroyed)
    {
        pthread_mutex_unlock(&queue->mtx);
        return -1;
    }

    int item = queue->items[queue->head];
    
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mtx);
    return item;
}

void task_queue_signal_destroy(TaskQueue* queue) {
    if (queue == NULL) {
        return;
    }

    queue->queue_is_destroyed = true;
    
    pthread_mutex_lock(&queue->mtx);
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mtx);
}

void task_queue_destroy(TaskQueue* queue) {
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    pthread_mutex_destroy(&queue->mtx);
    
    free(queue->items);
    free(queue);
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

    server->worker_threads = worker_create(worker_count, server);
    server->worker_count = worker_count;
    server->server_running = true;

    return server;
}

// Server is NULL after this
void server_close(Server* server) {
    task_queue_signal_destroy(server->task_queue);
    worker_destroy(server->worker_threads, server->worker_count);
    task_queue_destroy(server->task_queue);
    
    close(server->socket_fd);
    free(server->worker_threads);
    free(server);
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

// TODO: Handle errors
pthread_t* worker_create(int count, Server* server) {
    pthread_t* threads = malloc( sizeof(pthread_t) * count);

    for (int i = 0; i < count; i++)
    {
        pthread_create(threads + i, NULL, worker_routine, (void*)server);
    }
    
    return threads;
}

void worker_destroy(pthread_t* threads, int count) {
    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }
}

void interrupt_server(int) {
    g_server->server_running = false;
    shutdown(g_server->socket_fd, SHUT_RDWR);
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

int main(void) {
    Server* server = server_init("127.0.0.1", 6969, WORKERS_COUNT);
    if (server == NULL)
    {
        fprintf(stderr ,"Couldn't initialize server\n");
        return 1;
    }
    
    server_run(server);

    server_close(server);
    return 0;
}