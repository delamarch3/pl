MAKEFLAGS += --silent

CFLAGS=-I./include -Wall -Wextra -pedantic -std=c23 -g -Wno-char-subscripts -Wno-gnu-statement-expression-from-macro-expansion

SRC = $(wildcard src/*.c)
OBJS = $(SRC:src/%.c=build/%.o)
TARGET=main

run: main
	./main

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir build

clean:
	rm -rf build $(TARGET)
