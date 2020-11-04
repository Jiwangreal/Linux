.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=cli srv
all:$(BIN)
%.o:%.c
	$(cc) $(CFLAGS) -c $< -o $@
cli:cli.o sysutil.o
	$(cc) $(CFLAGS) $^ -o $@
srv:srv.o sysutil.o
	$(cc) $(CFLAGS) $^ -o $@
clean:
	rm -f *.o $(BIN)