#ifndef FTP_UTILS_H
#define FTP_UTILS_H

#define BUFFSIZE 4096
#define HTTP_NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n"
#define HTTP_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"

#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_MAGENTA "\x1b[35m"


#define ERROR_SEND_FILE             "Fail to send the file"
#define ERROR_OPEN_FILE             "Fail to open the file"
#define ERROR_SEND_FOLDER           "Fail to send the Directory"
#define ERROR_OPEN_FOLDER           "Fail to open the directory"
#define ERROR_FAIL_HTTP             "Fail to HTTP handler"
#define ERROR_NOT_FOUND_FILE_FOLDER "File or directory no found"
#define ERROR_INVALID_REQUEST       "Invalid Request"
#define ERROR_MISSING_ARGUMENTS     "You must expecify the port and the base directory like arguments"
#define ERROR_CONNECT_SOCKET "Fail to connect socket"
#define ERROR_BIND_SOCKET "Fail to bind socket"
#define ERROR_LISTING_SERVER "Fail to start listening"

#endif //FTP_UTILS_H
