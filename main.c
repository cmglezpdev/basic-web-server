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

#include "utils.h"
#include "buildpage.h"
#include "server.h"


void launch(struct Server *server) {
    char *buffer, *url;
    int address_length = sizeof(server -> address);

    while (1) {
        buffer = malloc(BUFFSIZE);
        url = malloc(BUFFSIZE);
        int client_socket = accept(server -> socket, (struct sockaddr *)&server -> address, (socklen_t *)&address_length);
        
        pid_t pid = fork();
        if(pid < 0) {
            fprintf(stderr, "%sFail to create a new child process%s\n", COLOR_RED, COLOR_RESET);
            exit(EXIT_FAILURE);
        }

        if(pid == 0) {
            close(server -> socket);

            recv(client_socket, buffer, BUFFSIZE, 0);
            sscanf(buffer, "GET %s ", url);
        
            if(url != NULL && strcmp(url, "/") == 0) 
                strcpy(url, server -> base_dir);
            if(url != NULL) url = replace(url, "%20", " ");

            if(url == NULL || http_handler(client_socket, url) != 0) {
                char *response = HTTP_NOT_FOUND;
                send(client_socket, response, strlen(response), 0);  
            }

            close(client_socket);
            free(buffer); free(url);

            // end child process
            exit(EXIT_SUCCESS); 
        } else {
            close(client_socket);
        }
    }
}

int main(int argc, char *args[]) {
    if(argc < 3) {
        fprintf(stderr, "%s%s%s\n", COLOR_RED, ERROR_MISSING_ARGUMENTS, COLOR_RESET);
        return EXIT_FAILURE;
    }

    int port  = atoi(args[1]);
    char* base_dir = args[2];

    struct Server server = server_contructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, port, 10, base_dir, launch);
    printf("Server launched in: %shttp://localhost:%d%s\n", COLOR_GREEN, port, COLOR_RESET);
    server.launch(&server);

    return EXIT_SUCCESS;
}