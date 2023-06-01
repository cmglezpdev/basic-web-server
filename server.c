#include "server.h"
#include "utils.h"
#include "buildpage.h"

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
        fprintf(stderr, "%s%s\n", ERROR_CONNECT_SOCKET, COLOR_RED);
        exit(EXIT_FAILURE);
    }

    if(bind(server.socket, (struct sockaddr *)&server.address, sizeof server.address) < 0) {
        fprintf(stderr, "%s%s\n", ERROR_BIND_SOCKET, COLOR_RED);
        exit(EXIT_FAILURE);
    }

    if( listen(server.socket, server.backlog) < 0 ) {
        fprintf(stderr, "%s%s\n", ERROR_LISTING_SERVER, COLOR_RED);
        exit(EXIT_FAILURE);
    }

    server.launch = launch;
    
    return server;
}

int send_file(int client_socket, char* path) {
    int file_fd = open(path, O_RDONLY);
    if(file_fd < 0) {
        return EXIT_FAILURE;
    } 
    
    off_t sent = 0;
    struct stat stat_buffer;
    fstat(file_fd, &stat_buffer);
    
    int i  = strlen(path) - 1;
    for(; path[i] != '/'; i --);
    char* filename = (path + i + 1);
    puts(filename);

    char header[BUFFSIZE];
    snprintf(header, sizeof(header), 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", filename, stat_buffer.st_size);

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
            fprintf(stderr, "%s%s%s", COLOR_RED, ERROR_SEND_FILE, COLOR_RESET);
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