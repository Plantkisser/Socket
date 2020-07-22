all:
	gcc -o serv -pthread server.c
	gcc -o client client.c