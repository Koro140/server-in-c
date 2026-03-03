#include "server.h"

#include <stdio.h>

#define WORKERS_COUNT 10
#define TASK_QUEUE_SIZE 100

void hello(int connection, const char* param) {
    const char msg[] = "Hello from Server\n";
    send(connection, msg, sizeof(msg), 0);
}

int main(void) {
    Server* server = server_init("127.0.0.1", 6969, WORKERS_COUNT, TASK_QUEUE_SIZE);
    if (server == NULL)
    {
        fprintf(stderr ,"Couldn't initialize server\n");
        return 1;
    }
    
    route_method_post(server->router, "/hello", hello);

    server_run(server);
    server_close(server);

    return 0;
}