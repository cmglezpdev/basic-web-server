#ifndef FTP_BUILDPAGE_H
#define FTP_BUILDPAGE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_SIZE_BUFFER 20480
#define BUFF_SIZE 1024

#define HTML_PAGE "./templates/page.html"
#define ITEM_TABLE_TEMPLATE "./templates/item.html"

#define HTTP_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define HTML_TABLE_HEAR "<!-- TABLE_HEAR -->"

char* build_page(DIR *dir, char* url);

#endif //FTP_BUILDPAGE_H
