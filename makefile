CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: simulator

simulator: main.o commands.o list.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c commands.h commands.c
	$(CC) $(CFLAGS) -c $< -o $@

commands.o: commands.c commands.h list.h
	$(CC) $(CFLAGS) -c $< -o $@

# list.o: list.c list.h
# 	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f commands.o main.o simulator


