CFLAGS=-Wall -Wextra -pedantic -std=c23 -g
OBJS=main.o string.o token.o parse.o
TARGET=main

run: main
	./main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
