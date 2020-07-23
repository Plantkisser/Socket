#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "Connection.h"







enum constants {
	BUFSIZE = 30,
	STD_PROTOCOL = 0,
	QUEUE_LEN = 30, 
	NO_FLAGS = 0
};

void* commands(void* data) 
{
	int* p_lsn_sck = data;

	char buf[BUFSIZE];
	while(1) {
		memset(buf, 0, BUFSIZE);
		scanf("%s", buf);
		if (strcmp("stop", buf) == 0) {
			unlink(SUN_PATH);
			close(*p_lsn_sck);
			exit(0);
		}
	}
}



void* response(void *data) {
	char buf[BUFSIZE];

	printf("Accepted successfully\n");

	int* sockfd = (int*) data;
	int count = 0;
	if  (recv(*sockfd, &count, sizeof(int), NO_FLAGS) != sizeof(int)) {
		printf("Cannot get amount of bytes\n");
		goto end;
	}	

	int res = 0;
	do {
		res = recv(*sockfd, buf, BUFSIZE, NO_FLAGS);
		if (res <= 0) {
			printf("Client is dead\n");

			goto end;
		}
		count-=res;

		send(*sockfd, buf, res, NO_FLAGS);
	} while(count != 0);

	send(*sockfd, "SERV", strlen("SERV"), NO_FLAGS);

	end:
	close(*sockfd);
	free(sockfd);

	return NULL;
}


int main() 
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGPIPE);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	int optval_true = 1;

	int lsn_sck = socket(AF_UNIX, SOCK_STREAM, STD_PROTOCOL);
	if (lsn_sck == -1) {
		perror("socket");
		exit((EXIT_FAILURE));
	}
	setsockopt(lsn_sck, SOL_SOCKET, SO_REUSEADDR, &optval_true, sizeof(optval_true));

	struct sockaddr_un lsn_addr;
	memset(&lsn_addr, 0, sizeof(lsn_addr));
	lsn_addr.sun_family = AF_UNIX;
	strncpy(lsn_addr.sun_path, SUN_PATH, sizeof(lsn_addr.sun_path) - 1);


	if  (bind(lsn_sck, &lsn_addr, sizeof(struct sockaddr_un)) != 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(lsn_sck, QUEUE_LEN) != 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	pthread_t tmp_thr;
	if (pthread_create(&tmp_thr, NULL, commands, &lsn_sck) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}


	while(1) {
		int sockfd = accept(lsn_sck, NULL, 0);
		if (sockfd == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		int* p_sck =(int*) calloc(1, sizeof(int));
		*p_sck = sockfd;

		pthread_create(&tmp_thr, NULL, response, p_sck);
	}

	close(lsn_sck);

	return 0;
}