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


#define BUFSIZE 4096

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
        perror("Fail to connect socket...\n");
        exit(EXIT_FAILURE);
    }

    if(bind(server.socket, (struct sockaddr *)&server.address, sizeof server.address) < 0) {
        perror("Fail to bind socket...\n");
        exit(EXIT_FAILURE);
    }

    if( listen(server.socket, server.backlog) < 0 ) {
        perror("Fail to start listening...\n");
        exit(EXIT_FAILURE);
    }

    server.launch = launch;
    
    return server;
}


static int send_page(int client_socket, const char *page) {
    char response[BUFSIZE];
    sprintf(response, "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s", strlen(page), page);
    int ret = send(client_socket, response, strlen(response), 0);
    return ret;
}

static int send_file(int client_socket, int file_fd, off_t file_size) {
    char buffer[BUFSIZE];
    ssize_t bytes_read, bytes_sent;
    off_t bytes_remaining = file_size;
    // Send the HTTP headers
    send(client_socket, "HTTP/1.1 200 OK\n", 16, 0);
    send(client_socket, "Content-Type: application/octet-stream\n", 45, 0);
    send(client_socket, "Content-Disposition: attachment\n\n", 33, 0);

    // Send the file data
    while ((bytes_read = read(file_fd, buffer, BUFSIZE)) > 0 && bytes_remaining > 0) {
        bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            return -1;
        }
        bytes_remaining -= bytes_sent;
    }

    return 0;
}


static int http_handler(int client_socket, const char *url) {
    struct stat file_stat;
    int file_fd;

    // Check if the URL points to a file
    if(stat(url, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
        // Open file
        file_fd = open(url, O_RDONLY);
        if(file_fd < 0) {
            send_page(client_socket, "<html><body><h1>Error al abrir el archivo</h1></body></html>");
            return -1;
        }

        // Send the file as a download
        send_file(client_socket, file_fd, file_stat.st_size);
        close(file_fd);
        return 0;
    }

    // Check if the URL points to a directory
    if(stat(url, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) {

        DIR *dir = opendir(url);
        if(!dir) {
            send_page(client_socket, "<html><body><h1>Error al abrir el directorio</h1></body></html>");
            return -1;
        }

        const char *page = "<html><body><h1>Directorio</h1><ul>%s</ul></body></html>";
        char *list = malloc(1); list[0] = '\0';
        char * item;
        struct dirent *ent;

        while((ent = readdir(dir)) != NULL) {
            // ignore directories . and ..
            if(ent->d_name[0] == '.') continue;

            item = malloc(strlen(ent->d_name) + BUFSIZE);
            sprintf(item, "<li><a href=\"%s\\%s\">%s</a></li>", url, ent->d_name, ent->d_name);
            list = realloc(list, strlen(list) + strlen(item) + strlen(url) + 10);
            strcat(list, item);
        }

        char *final_page = malloc(strlen(page) + strlen(list) + 10);
        sprintf(final_page, page, list);
        send_page(client_socket, final_page);
        
        if(item != NULL) free(item);
        free(list); free(final_page);
        closedir(dir);

        return 0;
    }

    // If the URL doesn't point to a file or a directory, return an error
    if (stat(url, &file_stat) != 0) {
        send_page(client_socket, "<html><body><h1>Archivo o directorio no encontrado</h1></body></html>");
        return -1;
    }

    return 0;
}

void launch(struct Server *server) {
    printf("======= WAITING FOR CONNECTION =======\n");

    char buffer[BUFSIZE];
    int address_length = sizeof(server -> address);
    int client_socket;

    while (1) {
        client_socket = accept(server -> socket, (struct sockaddr *)&server -> address, (socklen_t *)&address_length);
        read(client_socket, buffer, BUFSIZE);
      
        char *url = strtok(buffer, " ");
        url = strtok(NULL, " ");
        if (url == NULL) {
            printf("Invalid request\n");
            close(client_socket);
            continue;
        }

        if(strcmp(url, "/") == 0) url = server -> base_dir;
        if(http_handler(client_socket, url) < 0) {
            perror("Fail to HTTP handler\n");   
        }

        close(client_socket);
    }
}

int main(int argc, char *args[]) {
    if(argc < 3) {
        perror("You must expecify the port and the base directory like arguments\n");
        return 1;
    }

    int port  = atoi(args[1]);
    char* base_dir = args[2];

    struct Server server = server_contructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, port, 10, base_dir, launch);
    server.launch(&server);
    return 0;
}