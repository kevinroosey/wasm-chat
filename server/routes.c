#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void handle_login(int socket){
    char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nLogin\n";
    write(socket, response, strlen(response));
}

void handle_logout(int socket){
    char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nLogout\n";
    write(socket, response, strlen(response));
}

void handle_signup(int socket){
    char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nSignup\n";
    write(socket, response, strlen(response));
}

void handle_not_found(int socket){
    char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nNot Found\n";
    write(socket, response, strlen(response));
}