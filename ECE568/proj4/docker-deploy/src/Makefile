CC = g++
CFLAGS = -std=c++11 -pthread -Werror -Wall -pedantic -g

all: client server

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client
server: server.cpp
	$(CC) $(CFLAGS) server.cpp -o server
clean:
	rm client server
