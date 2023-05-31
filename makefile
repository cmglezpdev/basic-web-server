port ?= 8080
root ?= /home

dev:
	gcc main.c buildpage.c server.c utils.c -o main && ./main $(port) $(root)

build:
	gcc main.c buildpage.c server.c utils.c