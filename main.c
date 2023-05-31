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

#include "builtin_page.c"


#define BUFSIZE 4096
#define HTTP_NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n"
#define HTTP_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"


#pragma region Server

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

struct Server server_contructor(int domain, int service, int protocol, int interface, int port, int backlog, char* base_dir, void (*launch)(struct Server *server)) {
    struct Server server;
    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.base_dir = malloc(strlen(base_dir) + 1);
    strcpy(server.base_dir, base_dir);

    server.socket = socket(domain, service, protocol);

    if(server.socket < 0) {
        fprintf(stderr, "%sFail to connect socket...\n", COLOR_RED);
        exit(EXIT_FAILURE);
    }

    if(bind(server.socket, (struct sockaddr *)&server.address, sizeof server.address) < 0) {
        fprintf(stderr, "%sFail to bind socket...\n", COLOR_RED);
        exit(EXIT_FAILURE);
    }

    if( listen(server.socket, server.backlog) < 0 ) {
        fprintf(stderr, "%sFail to start listening...\n", COLOR_RED);
        exit(EXIT_FAILURE);
    }

    server.launch = launch;
    
    return server;
}

#pragma endregion

#pragma region Errors
struct MessagesError {
    char* sendFile;
    char* openFile;
    char* sendFolder;
    char* openFolder;
    char* noFoundFileFolder;
    char* invalidRequest;
    char* failHttp;
    char* missingArguments;
    
} messagesError;

void initialize_MessageError () {
    messagesError.sendFile = "Fail to send the file";
    messagesError.openFile = "Fail to open the file";
    messagesError.sendFolder = "Fail to send the Directory";
    messagesError.openFolder = "Fail to open the directory";
    messagesError.noFoundFileFolder = "File or directory no found";
    messagesError.invalidRequest = "Invalid Request";
    messagesError.failHttp = "Fail to HTTP handler";
    messagesError.missingArguments = "You must expecify the port and the base directory like arguments";
}

#pragma endregion

int send_file(int client_socket, char* path) {
    int file_fd = open(path, O_RDONLY);
    if(file_fd < 0) {
        return EXIT_FAILURE;
    } 
    
    off_t sent = 0;
    struct stat stat_buffer;
    fstat(file_fd, &stat_buffer);

    char header[BUFSIZE];
    snprintf(header, sizeof(header), 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", path, stat_buffer.st_size);

    send(client_socket, header, strlen(header), 0);
    sendfile(client_socket, file_fd, &sent, stat_buffer.st_size);
    close(file_fd);

    return EXIT_SUCCESS;
}

int http_handler(int client_socket, char *url) {
    struct stat file_stat;
    int file_fd;

    // Check if the URL points to a file
    if(stat(url, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
        file_fd = open(url, O_RDONLY);

        if (send_file(client_socket, url) < 0) {
            fprintf(stderr, "%s%s%s", COLOR_RED, messagesError.sendFile, COLOR_RESET);
            close(file_fd);
            return EXIT_FAILURE;
        }

        close(file_fd);
        return EXIT_SUCCESS;
    } 
    // Check if the URL points to a directory
    if(stat(url, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) {
        DIR *dir = opendir(url);
        char* page = build_page(dir, url);
        send(client_socket, page, strlen(page), 0);
        free(page); closedir(dir);
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

void launch(struct Server *server) {
    char *buffer, *url;
    int address_length = sizeof(server -> address);

    while (1) {
        buffer = malloc(BUFSIZE);
        url = malloc(BUFSIZE);
        int client_socket = accept(server -> socket, (struct sockaddr *)&server -> address, (socklen_t *)&address_length);
        
        recv(client_socket, buffer, BUFSIZE, 0);
        sscanf(buffer, "GET %s ", url);
      
        if(url != NULL && strcmp(url, "/") == 0) 
            strcpy(url, server -> base_dir);

        if(url == NULL || http_handler(client_socket, url) != 0) {
            char *response = HTTP_NOT_FOUND;
            send(client_socket, response, strlen(response), 0);  
        }

        close(client_socket);
        free(buffer); free(url);
    }
}


int main(int argc, char *args[]) {
    initialize_MessageError();
    
    if(argc < 3) {
        fprintf(stderr, "%s%s%s\n", COLOR_RED, messagesError.missingArguments, COLOR_RESET);
        return EXIT_FAILURE;
    }

    int port  = atoi(args[1]);
    char* base_dir = args[2];

    struct Server server = server_contructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, port, 10, base_dir, launch);
    printf("Server launched in: %shttp://localhost:%d%s\n", COLOR_GREEN, port, COLOR_RESET);
    server.launch(&server);
    return EXIT_SUCCESS;
}