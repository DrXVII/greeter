CC = gcc
LIBS = -lncurses
NAME = greeter.x86_64
DBG = -ggdb

all:
	$(CC) $(DBG) -o $(NAME) main.c $(LIBS)

dbg:
	$(CC) -o $(NAME) main.c $(LIBS)
