#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/ip.h>

int main(void) {
    int sock_fd = 0;
    struct sockaddr_in server_addr = {0};

    // Initializing server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6969);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        printf("Error ... couldn't create socket\n");
        return 1;
    }

    int err = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err == -1)
    {
        printf("Error ... couldn't connect to server\n");
        return 1;
    }
    
    close(sock_fd);
    return 0;
}