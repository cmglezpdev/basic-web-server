#include "builtin_page.h"

#define MAX_SIZE_BUFFER 10240
#define BUFF_SIZE 1024

#define HTML_PAGE "./templates/page.html"
#define ITEM_TABLE_TEMPLATE "./templates/item.html"

#define HTTP_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define HTML_TABLE_HEAR "<!-- TABLE_HEAR -->"

char **load_html_page() {
    int html_fs = open(HTML_PAGE, O_RDONLY);
    char buffer[MAX_SIZE_BUFFER];
    read(html_fs, buffer, MAX_SIZE_BUFFER);

    char *p = strstr(buffer, HTML_TABLE_HEAR);
    char **html = (char **)malloc(2 * sizeof(char *));

    html[1] = (char *)malloc(strlen(p) + 1);
    strcpy(html[1], p);

    buffer[p - buffer] = 0;
    html[0] = (char *)malloc(strlen(buffer) + 1);
    strcpy(html[0], buffer);

    close(html_fs);
    return html;
}

char* get_size(struct stat st) {
    char *size = (char *) malloc(100);
    if (!S_ISDIR(st.st_mode)) {
        unsigned long t = st.st_size / 1024;
        char *type;

        if (t > 1024) {
            t /= 1024;
            type = " mb";

            if (t > 1024) {
                t /= 1024;
                type = " gb";
            }
        } else {
            type = " kb";
        }

        sprintf(size, "%ld %s", t, type);
    } else {
        strcpy(size, "-");
    }

    return size;
}

char* get_last_date(struct stat st) {
    struct tm *tm = localtime(&st.st_mtime);
    char *date = (char *) malloc(64);

    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    return date;
}

char* build_html_page(DIR *dir, char* url) {
    char **html = load_html_page();
    char *page = (char *) malloc(2 * MAX_SIZE_BUFFER);
    char *item_template = (char *) malloc(BUFF_SIZE);

    int item_template_fd = open(ITEM_TABLE_TEMPLATE, O_RDONLY);
    read(item_template_fd, item_template, BUFF_SIZE);
    close(item_template_fd);

    strcpy(page, HTTP_HEADER);
    strcat(page, html[0]);

    struct stat st;
    struct dirent *ent;

    while((ent = readdir(dir)) != NULL) {
        if(ent -> d_name[0] == '.') continue;

        char* item_url = (char *)malloc(strlen(url) + strlen(ent -> d_name) + 6);
        strcpy(item_url, url); strcat(item_url, "/"); strcat(item_url, ent -> d_name);
        stat(item_url, &st);

        char *type = S_ISDIR(st.st_mode) ? "folder" : "file";
        char *size = get_size(st);
        char *date = get_last_date(st);
        char *item = (char *) malloc(strlen(item_template) + strlen(url) + strlen(ent -> d_name) + 100);

        sprintf(item, item_template, item_url, type, ent -> d_name, size, date);
        strcat(page, item);
        
        free(size); free(date); free(item); free(item_url);

        if(2 * MAX_SIZE_BUFFER - strlen(page) < MAX_SIZE_BUFFER) {
            page = (char *) realloc(page, 4 * MAX_SIZE_BUFFER);
        }
    }

    strcat(page, html[1]);
    free(html); free(item_template);

    return page;
}

char *render(DIR *dir, char* url) {
    char* page = build_html_page(dir, url);
    return page;
}