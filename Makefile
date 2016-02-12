TARGET = program3
OBJ_FILES = \
  source.o

CFLAGS = -g -Wall 

CC = g++ 

$(TARGET):  	$(OBJ_FILES)
		$(CC) -g -Wall $(OBJ_FILES) -o $@
