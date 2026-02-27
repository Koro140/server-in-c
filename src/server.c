#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/ip.h>

int main(void) {
    int sock_fd = 0;
    int connection_fd = 0;

    struct sockaddr_in sock_addr = {0};
    struct sockaddr_in peer_addr = {0};
    socklen_t peer_addr_len = {0};

    // Initializing host address
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(6969);
    sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        printf("Error ... couldn't create socket\n");
        return 1;
    }

    // I don't understand this but make running program easier
    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    int err = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (err == -1)
    {
        printf("Error ... couldn't bind socket\n");
        close(sock_fd);
        return 1;
    }
    
    err = listen(sock_fd, 1);
    if (err == -1)
    {
        printf("Error ... listining error\n");
        close(sock_fd);
        return 1;
    }

    connection_fd = accept(sock_fd, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if (connection_fd == -1)
    {
        printf("Error ... couldn't accept connection\n");
        close(sock_fd);
        return 1;
    }

    close(connection_fd);
    close(sock_fd);
    return 0;
}