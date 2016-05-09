CC = g++
CFLAGS = -std=c++14 -Wall -Wextra -g

all: simple_shell

simple_shell: shell.o cmd_parser.o
	$(CC) $(CFLAGS) $^ -o $@ 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o simple_shell cmd_parser_test

cmd_parser_test: cmd_parser.cpp
	$(CC) $(CFLAGS) -D__LIB_TEST__ $< -o $@

