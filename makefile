CC = gcc
LIBS = -lncurses
NAME = greeter.x86_64

all:
	$(CC) -o $(NAME) main.c $(LIBS)

dbg:
	$(CC) -ggdb -o $(NAME) main.c $(LIBS)
