#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "routes.c"

#define PORT 8080
// Revisit buffer size once we know more
#define BUFFER_SIZE 1024

int create_socket() {
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}


void bind_socket(int server_fd) {
   struct sockaddr_in address;
   int address_len = sizeof(address);
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);

   if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
     perror("bind failed");
     exit(EXIT_FAILURE);
   }
}

void listener(int server_fd) {
    if(listen(server_fd, 3) < 0) {
        perror("listening");
	exit(EXIT_FAILURE);
    }
}

int accept_connections(int server_fd){
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0) {
        perror("Cannot accept connection");
	exit(EXIT_FAILURE);
    }
    return new_socket;
}



void route_request(int socket, const char *method, const char *path) {
    if (strcmp(path, "/login") == 0) {
        handle_login(socket);
    } else if (strcmp(path, "/logout") == 0) {
        handle_logout(socket);
    } else if (strcmp(path, "/signup") == 0) {
        handle_signup(socket);
    } else {
        handle_not_found(socket);
    }
}

void handle_client(int socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(socket, buffer, BUFFER_SIZE);
    printf("Request: %s\n", buffer);
    char method[10], path[100];

    sscanf(buffer, "%s %s", method, path);
    printf("Method: %s %s\n", method, path);

    route_request(socket, method, path);
}

void close_socket(int socket){
    close(socket);
}

int main() {
    int server_fd = create_socket();
    bind_socket(server_fd);
    listener(server_fd);
    while(1) {
        int new_socket = accept_connections(server_fd);
        handle_client(new_socket);
        close_socket(new_socket);
    }
    return 0;
}

