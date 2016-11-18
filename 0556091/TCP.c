#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TCP.h"

int passiveTCP(char* portNumber)
{
	int svsockfd, clsockfd;
	char clIP[INET_ADDRSTRLEN];
	struct sockaddr_in svaddr, claddr;
	socklen_t addrLen = sizeof(struct sockaddr_in);

	//set sockaddr_in struct
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(atoi(portNumber));
	svaddr.sin_addr.s_addr = INADDR_ANY;

	//create TCP connection socket
	if ((svsockfd = socket(svaddr.sin_family, SOCK_STREAM, 0)) == -1)
		perror("socket\n");
	if (bind(svsockfd, (struct sockaddr*) &svaddr, sizeof(svaddr)) == -1)
		perror("bind\n");
	if (listen(svsockfd, BACKLOG) == -1)
		perror("listen\n");
	else
		puts("waiting for connection ... ");
	for (;;) {
		//wait for connections
		if((clsockfd = accept(svsockfd, (struct sockaddr*) &claddr, &addrLen)) == -1) {
			perror("accept\n");
			exit(EXIT_FAILURE);
		} else {
			if(fork() > 0) {
				close(clsockfd);
				continue;
			}
			printf("Connection from %s is accepted, SocketFD = %d\n",\
			       inet_ntop(svaddr.sin_family, &claddr.sin_addr.s_addr, clIP,\
			                 sizeof(clIP)), clsockfd);

			return clsockfd;
		}
	}

}


