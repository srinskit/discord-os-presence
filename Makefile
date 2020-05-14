CC=gcc
TARGET=main
INCLUDE = ./include
ARCH= $(shell uname -m)
LIB= ./lib/$(ARCH)
LIBS= -L$(LIB) -l:discord_game_sdk.so -Wl,-rpath,"$(LIB)" -lm

all:
	$(CC) main.c -o $(TARGET) $(LIBS) -I$(INCLUDE) 

clean:
	rm $(TARGET)
