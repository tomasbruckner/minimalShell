#	Name: Tomas Bruckner, tomasbrucknermail@gmail.com
#	Date: 2016-04-16
#	Description:
#

CC=gcc
CFLAGS=-O -Wall -pedantic -std=c99

shell:
	$(CC) $(CFLAGS) shell.c -o shell -lpthread
clean:
	rm -f shell
