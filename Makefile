CFLAGS=-Wall -Wextra -pedantic -std=c23 -g -Wno-char-subscripts -Wno-gnu-statement-expression-from-macro-expansion
OBJS=main.o string.o token.o parse.o gen.o
TARGET=main

run: main
	./main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
