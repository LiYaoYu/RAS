#include <stdio.h>
#include <stdlib.h>

#include "TCP.h"
#include "RAS.h"

int main(int argc, char* argv[], char* environ[])
{
	int svsockfd, clsockfd;

	changeFolder("../ras");

	if (argc != 2) {
		puts("Usage : ./server <port number>");
		return 0;
	} else {
		clsockfd = passiveTCP(argv[1]); //parent is wait for accept in passiveTCP
	}

	init_environ();
	clientHelper(clsockfd);

}