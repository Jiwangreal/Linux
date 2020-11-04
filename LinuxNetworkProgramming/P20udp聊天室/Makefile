.PHONY:clean all
CC=g++
CFLAGS=-Wall -g
BIN=chatcli chatsrv
all:$(BIN)
%.o:%.cpp
	$(cc) $(CFLAGS) -c $< -o $@
chatcli:chatcli.o
	$(cc) $(CFLAGS) $^ -o $@
chatsrv:chatsrv.o
	$(cc) $(CFLAGS) $^ -o $@
clean:
	rm -f *.o $(BIN)