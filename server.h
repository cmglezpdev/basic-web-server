#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

struct Server {
    int domain;
    int service;
    int protocol;
    int interface;
    int port;
    int backlog;
    char* base_dir;

    struct sockaddr_in address;

    int socket;    

    void (*launch)(struct Server *server);
};

struct Server server_contructor(int domain, int service, int protocol, int interface, int port, int backlog, char* base_dir, void (*launch)(struct Server *server));

int send_file(int client_socket, char* path);

int http_handler(int client_socket, char *url);

#endif //FTP_SERVER_H
