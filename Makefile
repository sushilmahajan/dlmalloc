all:
	gcc main.c malloc.c bins.c sys_calls.c debug.c -Wall -lm -o project
