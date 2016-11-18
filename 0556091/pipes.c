#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pipes.h"
#include "RAS.h"


int* newPipe(int* pipefd)
{
	if (pipe(pipefd) == -1)
		perror("create new pipe");

	return pipefd;
}

void pipeDup(int* fd, int action)
{
	printf("fd = %d, %d, unread = %d\n", fd[0], fd[1], unread);

	if (action == PIPE2IN) {
		if (dup2(unread, STDIN_FILENO) == -1)
			perror("pipeDup PIPE2IN");//puts("pipeDup PIPE2IN");
		else {
			if(unread != STDIN_FILENO)
				close(unread);
		}
	} else if (action == PIPE2OUT) {
		puts("PIPE2OUT before dup2");
		if (dup2(fd[1], STDOUT_FILENO) == -1 || dup2(fd[1], STDERR_FILENO) == -1) {
			puts("dup2 error");
		} else {
			if(fd[1] != STDOUT_FILENO && fd[1] != STDERR_FILENO)
				close(fd[1]);
		}
	} else { //PIPE2NEXT
		if (dup2(unread, STDIN_FILENO) == -1)
			puts("pipeDup PIPE2NEXT in");
		else {
			if (unread != STDIN_FILENO)
				close(unread);
		}

		if (dup2(fd[1], STDOUT_FILENO) == -1 || dup2(fd[1], STDERR_FILENO) == -1)
			puts("pipeDup PIPE2NEXT");
		else {
			if(fd[1] != STDOUT_FILENO && fd[1] != STDERR_FILENO)
				close(fd[1]);
		}
	}
}


void pipeNumber(int pipeInfd, int pipeTo, int action)
{
	PNNode *ptr;

	if(action == PIPENORE) { //should be called only when pipeNumberTable[0] != NULL
		int buffIndex = 0;
		char buffer[BUFFSIZE + 1];

		ptr = pipeNumberTable[0];
		while (ptr) {
			buffIndex += read(ptr->Infd, &buffer[buffIndex], (sizeof(char) * (BUFFSIZE - buffIndex + 1)));

			ptr = ptr->pNext;
		}
		buffer[buffIndex] = '\0';

		int *rcv_pre_pipe = malloc(sizeof(int) * 2);
		rcv_pre_pipe = newPipe(rcv_pre_pipe);

		printf("rcv_pre_pipe = %d, %d\n", rcv_pre_pipe[0], rcv_pre_pipe[1]);

		if(write(rcv_pre_pipe[1], buffer, strlen(buffer)) <= 0) {
			puts("write to rcv_pre_pipe in pipeNumber");
			close(rcv_pre_pipe[0]);
		}

		close(rcv_pre_pipe[1]);

		unread = rcv_pre_pipe[0];

		/*
		if(dup2(rcv_pre_pipe[0], STDIN_FILENO) == -1) {
			puts("dup2 in pipeNumber");
			exit(EXIT_FAILURE);
		} 
		*/
		/*else { because rcv_pre_pipe[0] == STDIN_FILENO 
			close(rcv_pre_pipe[0]);
		} */

	} else if(action == PIPENOWR) {
		if (!pipeNumberTable[pipeTo]) { //no previous pipe to pipeTo
			pipeNumberTable[pipeTo] = malloc(sizeof(struct pipeNumberNode));
			ptr = pipeNumberTable[pipeTo];
		} else { 
			ptr = pipeNumberTable[pipeTo];

			while (ptr->pNext) {
				printf("ptr->Infd = %d\n", ptr->Infd);
				ptr = ptr->pNext;
			}
			ptr->pNext = malloc(sizeof(struct pipeNumberNode));
			ptr = ptr->pNext;
		}
		ptr->Infd = pipeInfd;
		ptr->pNext = NULL;
	} else if(action == PIPENOSH) {
		for (int i = 0; i < (MAXPIPENUM - 1); i++){
			pipeNumberTable[i] = pipeNumberTable[i + 1];			
		}
		pipeNumberTable[MAXPIPENUM - 1] = NULL;
	} else { //PIPENOCL (parent do this)
		if (pipeNumberTable[0]) {
			PNNode *tmp;
			ptr = pipeNumberTable[0];
			while(ptr) { //close pipefd and free nodes
				close(ptr->Infd);
				tmp = ptr;
				ptr = ptr->pNext;
				free(tmp);
			}
		}
	}
}
