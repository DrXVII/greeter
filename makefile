CC = gcc
LL = gcc
LIBS = -lncurses
NAME = greeter.x86_64
DBG_F = -ggdb
O_DIR = o
INC_DIR = inc
INC_F = -I $(INC_DIR)
SRC_DIR = src
OBJ = o/main.o \
      o/config.o

all: $(NAME)

$(NAME): $(O_DIR) $(OBJ)
	$(LL) -o $(NAME) $(OBJ) $(LIBS)

o:
	mkdir -p o

$(O_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(DBG_F) $(INC_F) -c -o $@ $<

clean:
	rm -v o/*.o

#- release ---------------------------------------------------------------------
#TODO do release script below (old style is deprecated)
release:
	$(CC) -o $(NAME) main.c $(LIBS)
