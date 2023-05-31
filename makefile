port ?= 8080
root ?= /home

dev:
	gcc main.c -o main && ./main $(port) $(root)

build:
	gcc main.c -o main