#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/tcp.h>




enum constants {
	BUFSIZE = 100,
	NO_FLAGS = 0,
	STD_PROTOCOL = 0,
	O_FLAGS = 0
};


int main() {
	int optval_true = 1;
	
	int br_sck = socket(AF_INET, SOCK_DGRAM, STD_PROTOCOL);
	if (br_sck == -1) {
		printf("Cannot create broadcast socket\n");
		exit(0);
	}
	setsockopt(br_sck, SOL_SOCKET, SO_BROADCAST, &optval_true, sizeof(optval_true));
	setsockopt(br_sck, SOL_SOCKET, SO_REUSEADDR, &optval_true, sizeof(optval_true));

	struct sockaddr_in br_addr;
	br_addr.sin_family = AF_INET;
	br_addr.sin_port = htons(50000);
	br_addr.sin_addr.s_addr = INADDR_ANY;

	bind(br_sck, (struct sockaddr *) &br_addr, sizeof(br_addr));

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	char buf[BUFSIZE];
	socklen_t tmp_size = sizeof(serv_addr);
	recvfrom(br_sck, buf, sizeof(buf), NO_FLAGS, (struct sockaddr *) &serv_addr, &tmp_size);
	close(br_sck);

	serv_addr.sin_port = htons(50000);
	serv_addr.sin_family = AF_INET;


	int mn_sck = socket(AF_INET, SOCK_STREAM, STD_PROTOCOL);
	if (mn_sck == -1) {
		printf("Cannot create main socket\n");
		close(br_sck);
		exit(0);
	}
	connect(mn_sck, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	setsockopt(mn_sck, SOL_SOCKET, SO_KEEPALIVE, &optval_true, sizeof(optval_true));
	int cnt = 2;
	setsockopt(mn_sck, SOL_SOCKET, TCP_KEEPCNT, &cnt, sizeof(cnt));
	int idle = 10;
	setsockopt(mn_sck, SOL_SOCKET, TCP_KEEPIDLE, &idle, sizeof(idle));
	int intvl = 5;
	setsockopt(mn_sck, SOL_SOCKET, TCP_KEEPINTVL, &intvl, sizeof(intvl));




	memset(buf, 0, BUFSIZE);

	printf("Write message:\n");
	scanf("%s", buf);

	send(mn_sck, buf, strlen(buf), NO_FLAGS);

	memset(buf, 0, BUFSIZE);
	recv(mn_sck, buf, BUFSIZE, NO_FLAGS);

	printf("Your message:\n%s\n", buf);

	shutdown(mn_sck, SHUT_RDWR);

	return 0;
}