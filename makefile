CC = gcc
LL = gcc
LIBS = -lncurses
NAME = greeter.x86_64
DBG_F = -ggdb
O_DIR = o
INC_DIR = inc
INC_F = -I $(INC_DIR)
SRC_DIR = src
OBJ = $(O_DIR)/main.o \
      $(O_DIR)/config.o

all: $(NAME)

$(NAME): $(O_DIR) $(OBJ)
	$(LL) -o $@ $(OBJ) $(LIBS)

$(O_DIR):
	mkdir -p $@

$(O_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(DBG_F) $(INC_F) -c -o $@ $<

clean:
	rm -v $(O_DIR)/*.o
	rm $(NAME)

#- release ---------------------------------------------------------------------
DIR_REL = release_dir
NAME_REL = $(DIR_REL)/$(NAME)
O_DIR_REL = $(DIR_REL)/$(O_DIR)
OBJ_REL = $(O_DIR_REL)/main.o \
          $(O_DIR_REL)/config.o
CFG_FILES = $(DIR_REL)/config.cfg \
	    $(DIR_REL)/daily_tips.adoc \
	    $(DIR_REL)/todo.adoc

release: $(NAME_REL)

$(NAME_REL): $(O_DIR_REL) $(O_DIR_REL) $(OBJ_REL) $(CFG_FILES)
	$(CC) -o $@ $(OBJ_REL) $(LIBS)

$(O_DIR_REL)/%.o: $(SRC_DIR)/%.c
	$(CC) $(INC_F) -c -o $@ $<

$(DIR_REL)/config.cfg: config.cfg
	cp $< $@

$(DIR_REL)/daily_tips.adoc: daily_tips.adoc
	cp $< $@

$(DIR_REL)/todo.adoc: todo.adoc
	cp $< $@


$(O_DIR_REL):
	mkdir -p $@

$(DIR_REL):
	mkdir -p $@
