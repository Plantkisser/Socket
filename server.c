#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/tcp.h>







enum constants {
	BUFSIZE = 100,
	STD_PROTOCOL = 0,
	QUEUE_LEN = 30, 
	NO_FLAGS = 0
};



struct accept_inf {
	int sockfd;
	struct sockaddr addr;
	socklen_t addrlen;
};


void* response(void *data) {
	char buf[BUFSIZE];

	printf("Accepted successfully\n");

	int* sockfd = (int*) data;

	int count = 0;
	do {
		count = recv(*sockfd, buf, BUFSIZE, NO_FLAGS);
		send(*sockfd, buf, count, NO_FLAGS);
	} while(count == BUFSIZE);
	shutdown(*sockfd, SHUT_RDWR);
	free(sockfd);

	return NULL;
}


int main() {
	int optval_true = 1;

	int lsn_sck = socket(AF_INET, SOCK_STREAM, STD_PROTOCOL);
	if (lsn_sck == -1) {
		printf("Cannot create socket\n");
		exit(0);
	}
	setsockopt(lsn_sck, SOL_SOCKET, SO_REUSEADDR, &optval_true, sizeof(optval_true));

	struct sockaddr_in lsn_addr;
	lsn_addr.sin_family = AF_INET;
	lsn_addr.sin_port = htons(50000);
	lsn_addr.sin_addr.s_addr = INADDR_ANY;



	if  (bind(lsn_sck, &lsn_addr, sizeof(struct sockaddr_in)) != 0) {
		printf("Cannot bind listen socket\n");
		exit(0);
	}
	if (listen(lsn_sck, QUEUE_LEN) != 0) {
		printf("Listen error\n");
		exit(0);
	}

	int br_sck = socket(AF_INET, SOCK_DGRAM, STD_PROTOCOL);
	setsockopt(br_sck, SOL_SOCKET, SO_BROADCAST, &optval_true, sizeof(optval_true));

	struct sockaddr_in br_addr;
	br_addr.sin_family = AF_INET;
	br_addr.sin_port = htons(50000);
	br_addr.sin_addr.s_addr = INADDR_BROADCAST;



	fcntl(lsn_sck, F_SETFL, O_NONBLOCK);
	while(1) {
		int smth = 0;
		sendto(br_sck, &smth, sizeof(smth), NO_FLAGS, (struct sockaddr*) &br_addr, sizeof(br_addr));

		
		
		int sockfd = accept(lsn_sck, NULL, 0);
		if (sockfd == -1) {
			sleep(1);
			continue;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval_true, sizeof(optval_true));
		int cnt = 2;
		setsockopt(sockfd, SOL_SOCKET, TCP_KEEPCNT, &cnt, sizeof(cnt));
		int idle = 10;
		setsockopt(sockfd, SOL_SOCKET, TCP_KEEPIDLE, &idle, sizeof(idle));
		int intvl = 5;
		setsockopt(sockfd, SOL_SOCKET, TCP_KEEPINTVL, &intvl, sizeof(intvl));


		int* sck =(int*) calloc(1, sizeof(int));
		*sck = sockfd;
		

		pthread_t tmp;

		pthread_create(&tmp, NULL, response, sck);
	}

	close(lsn_sck);

	return 0;
}