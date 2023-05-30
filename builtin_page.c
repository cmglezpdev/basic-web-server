#include "builtin_page.h"

#define MAX_SIZE_BUFFER 10240
#define HTML_PAGE "./templates/page.html"
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

void build_table_name(char *html_response, char *url, char *file, struct stat st) {
    char* block = (char *) malloc(1024);
    sprintf(block, "<td><a href=\"%s\">", url);
    strcat(html_response, block);

    if (S_ISDIR(st.st_mode)) {
        strcat(html_response, "<span><div class=\"folder\"></div> ");
    } else {
        strcat(html_response, "<span><div class=\"file\"></div> ");
    }
    strcat(html_response, file);

    strcat(html_response, "</span></a>");
    strcat(html_response, "</td>");

    free(block);
}

void build_table_size(char *html_response, struct stat st) {
    strcat(html_response, "<td class=\"center\">");

    char aux[64];
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

        sprintf(aux, "%ld", t);
        strcat(html_response, aux);
        strcat(html_response, type);
    } else {
        strcat(html_response, "-");
    }

    strcat(html_response, "</td>");
}

void build_table_last_date(char *html_response, struct stat st) {
    strcat(html_response, "<td class=\"center\">");

    struct tm *tm;

    tm = localtime(&st.st_mtime);

    char aux[64];
    sprintf(aux, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    strcat(html_response, aux);
    strcat(html_response, "</td>");
}

char* build_html_page(DIR *dir, char* url) {
    char **html = load_html_page();
    char *page = (char *) malloc(2 * MAX_SIZE_BUFFER);
    strcpy(page, HTTP_HEADER);
    strcat(page, html[0]);

    struct stat st;
    struct dirent *ent;

    while((ent = readdir(dir)) != NULL) {
        if(ent -> d_name[0] == '.') continue;

        char* item_url = (char *)malloc(strlen(url) + strlen(ent -> d_name) + 6);
        strcpy(item_url, url); strcat(item_url, "/"); strcat(item_url, ent -> d_name);

        stat(item_url, &st);
        strcat(page, "<tr>");
        build_table_name(page, item_url, ent->d_name, st);
        build_table_size(page, st);
        build_table_last_date(page, st);
        strcat(page, "</tr>");
    

        if(2 * MAX_SIZE_BUFFER - strlen(page) < MAX_SIZE_BUFFER) {
            page = (char *) realloc(page, 4 * MAX_SIZE_BUFFER);
        }
    }

    strcat(page, html[1]);
    free(html);

    return page;
}

char *render(DIR *dir, char* url) {
    char* page = build_html_page(dir, url);
    return page;
}