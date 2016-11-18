#ifndef _PIPES_H
#define _PIPES_H

#define MAXPIPENUM 1000

#define PIPE2IN 0
#define PIPE2OUT 1
#define PIPE2NEXT 2

#define PIPENORE 0 //READ
#define PIPENOWR 1 //WRITE
#define PIPENOSH 2 //SHIFT
#define PIPENOCL 3 //CLOSE

typedef struct pipeNumberNode {
	int Infd;
	struct pipeNumberNode *pNext;
} PNNode;

PNNode* pipeNumberTable[MAXPIPENUM];
int unread;

int* newPipe(int* pipefd);

void pipeDup(int* pipefd, int action);

void pipeNumber(int pipeInfd, int pipeTo, int action);

#endif
