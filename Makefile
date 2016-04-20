CC=gcc
CFLAGS=-O -Wall -std=c99

shell:
	$(CC) $(CFLAGS) shell.c -o shell -lpthread
clean: 
	rm -f shell
