SRC = src/main.cpp

CC = clang++

COMPILER_FLAGS = -w -I/usr/include

LINKER_FLAGS = -L/usr/local/lib -lSDL2 -mcmodel=medium

OBJ_NAME = Lemon-16
OBJ_PATH = bin/$(OBJ_NAME)

build :
	$(CC) $(SRC) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_PATH)

test :
	./$(OBJ_PATH)

buildtest :
	$(CC) $(SRC) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_PATH)
	./$(OBJ_PATH)
