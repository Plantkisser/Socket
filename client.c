#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "Connection.h"




enum constants {
	BUFSIZE = 4000,
	NO_FLAGS = 0,
	STD_PROTOCOL = 0,
	O_FLAGS = 0,
	STDIN = 0
};


int main() 
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGPIPE);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	struct sockaddr_un serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, SUN_PATH, sizeof(serv_addr.sun_path) - 1);

	int mn_sck = socket(AF_UNIX, SOCK_STREAM, STD_PROTOCOL);
	if (mn_sck == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if (connect(mn_sck, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}



	char buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);

	printf("Write message:\n");
	read(STDIN, buf, BUFSIZE);

	int len = strlen(buf);
	send(mn_sck, &len, sizeof(len), NO_FLAGS);
	send(mn_sck, buf, strlen(buf), NO_FLAGS);


	memset(buf, 0, BUFSIZE);

	len += 4; // beccause serv concatenate "SERV" in the end 
	int count = 0;
	printf("Your message:\n");
	int res = 0;
	while(count != len) {
		memset(buf, 0, BUFSIZE);
		res = recv(mn_sck, buf, BUFSIZE, NO_FLAGS);
		if (res == 0)
		{
			printf("\nServer is dead\n");
			return 0;
		}
		count += res;
		printf("%s", buf);
	}
	printf("\n");

	close(mn_sck);

	return 0;
}