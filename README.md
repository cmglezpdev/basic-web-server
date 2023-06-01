# A Web Server in C
> University Project

This web server is a __ftp__ that show the folders and files from the pc.

## Run Command

You can execute the command `make` in the console and the server starting with __PORT=8080__ and __ROOT='/home'__ by default 

```bash
make
```

If you want to change the port or the the root directory you can execute the command:

``` bash
gcc main.c buildpage.c server.c utils.c -o main
# and after
./main <port> <root path>
```

o directly

``` bash
gcc main.c buildpage.c server.c utils.c -o main && ./main <port> <root path>
```

where `<port>` and `<root path>` are the port and the root path respectly.