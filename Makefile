#
#	File: Makefile
#	Course: POS - Advanced Operating Systems
#	Project: 2. Shell
#	Name: Tomas Bruckner, xbruck02@stud.fit.vutbr.cz
#	Date: 2016-04-16
#	Description:
#

CC=gcc
CFLAGS=-O -Wall -pedantic -std=c99

shell:
	$(CC) $(CFLAGS) shell.c -o shell -lpthread
clean:
	rm -f shell
