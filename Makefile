all:server client

client:client_16017.c
	gcc -pthread -w client_16017.c -o client

server:server_16017.c
	gcc -pthread -w server_16017.c -o server

clean:
	rm -f server
	rm -f client
