all: server.o client.o

server.o: server.c tools.h
	gcc server.c -o server

client.o: client.c tools.h
	gcc client.c -o client

clean:
	rm *.o client server