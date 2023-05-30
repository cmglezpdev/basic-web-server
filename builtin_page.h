
#ifndef BUILTIN_PAGE
#define BUILTIN_PAGE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

char* render(DIR *dir, char *url);

#endif