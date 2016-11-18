#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#include "RAS.h"
#include "pipes.h"

extern PNNode* pipeNumberTable[MAXPIPENUM];
int numChild = 0;

void changeFolder(char* folder)
{
	if(chdir(folder) < 0) {
		perror("chdir()\n");
		exit(EXIT_FAILURE);
	}
}


void init_environ()
{
	if (clearenv() != 0)
		puts("Error : clearenv()\n");
	if (setenv(ENVPATH_NAME, ENVPATH_VALUE, ENV_OVERWR) != 0) {
		puts("Error : setenv\n");
		exit(EXIT_FAILURE);
	}
}


void welcomemsg(int clsockfd)
{
	char* welcome_str = "****************************************\n"\
	                    "** Welcome to the information server. **\n"\
	                    "****************************************\n% ";
	//send welcome msg
	if (write(clsockfd, welcome_str, strlen(welcome_str)) == -1) {
		puts("Error : Cannot send message to client.\n");
		exit(EXIT_FAILURE);
	}
	return;
}


void clientHelper(int clsockfd)
{
	welcomemsg(clsockfd);

	get_and_execute_msg(clsockfd);
}


void get_and_execute_msg(int clsockfd)
{
	int numRead = 0;
	char clInput[BUFFSIZE + 1];

	while ((numRead += read(clsockfd, &clInput[numRead], sizeof(clInput))) >= 0) {
		if (clInput[numRead - 1] != '\n') { //to ensure the msg is read completely
			continue;
		} else {
			if (!strtok(clInput, "\r\n")) { //when the client input is nothing
				numRead = 0;
				memset(clInput, '\0', sizeof(clInput));
				prompt(clsockfd);
				continue;
			}

			numRead = strlen(clInput);
			clInput[numRead] = '\0';

			printf("got the msg : %s\n", clInput);

			//clean zombie
			while (numChild > 0) {
				wait(NULL);
				numChild--;
			}

			pipeNumber(0, 0, PIPENOSH); //shift table
			cmdHelper(clsockfd, clInput);

		}
		numRead = 0;
		memset(clInput, '\0', sizeof(clInput));
		prompt(clsockfd);
	}
	return;
}


void prompt(int clsockfd)
{
	if (write(clsockfd, "% ", strlen("% ")) == -1)
		puts("prompt error");
}


void cmdHelper(int clsockfd, char* clInput)
{
	int numPartition;
	int numCmds;
	int round = 0;
	char *response;
	char *CutByPipe[BUFFSIZE + 1];
	char *cmds[MAXCMDSIZE + 1];

	if (isIllegal(clInput)) {
		response = "'/' is illegal.";
		write(clsockfd, response, strlen(response));
		return;
	}

	numPartition = parseInput(clInput, CutByPipe, "|!");

	numCmds = parseInput(CutByPipe[0], cmds, " "); //get the executable cmds

	if (strcmp("exit", cmds[0]) == 0)
		exitHelper(clsockfd);
	else if (strcmp("printenv", cmds[0]) == 0) {
		printenvHelper(clsockfd, cmds, numCmds);
		return;
	} else if (strcmp("setenv", cmds[0]) == 0) {
		setenvHelper(clsockfd, cmds, numCmds);
		return;
	}

	int pid;
	int *pipefd = malloc(sizeof(int) * 2);

	pipefd = newPipe(pipefd);
	unread = STDIN_FILENO;

	while (round < numPartition) {  //totally numPartition rounds
		pid = fork();
		numChild++;

		if (pid < 0)
			perror("fork in cmdHelper");
		else if (pid == 0) { //child
			execChild(clsockfd, cmds, pipefd, round, numPartition);
		} else { //parent

			//pipefd create by parent
			pipefd = execParent(pipefd, cmds, round, numPartition);
			if (!pipefd) //unknown cmd occurs
				break;
		}
		round++;
		numCmds = parseInput(CutByPipe[round], cmds, " "); //get the next executable cmds
	}
}


int isIllegal(char* clInput)
{
	char *cntPtr;

	for (cntPtr = clInput; *cntPtr != '\0'; cntPtr++) {
		if (*cntPtr == '/')
			return 1;
	}
	return 0;
}


void exitHelper(int clsockfd)
{
	printf("client %d left.", clsockfd);
	close(clsockfd);
	exit(EXIT_SUCCESS);
}


void printenvHelper(int clsockfd, char* cmds[], int numCmds)
{
	char *response;
	char **envPtr;
	response = malloc(sizeof(char) * (BUFFSIZE + 1));
	response = "Usage : printenv <ENV_name>\n";

	if(numCmds == 2) {
		for (envPtr = environ; *envPtr != NULL; envPtr++) {
			char *tmp = strdup(*envPtr);
			if (strcmp(strtok(tmp, "="), cmds[1]) == 0) {
				response = strcat(strcat(strcat(cmds[1], "="), getenv(cmds[1])),"\n");
				write(clsockfd, response, strlen(response));
				return;
			}
		}
		response = strcat(cmds[1], " is not in the ENV_List\n");
	}
	write(clsockfd, response, strlen(response));
	return;
}


void setenvHelper(int clsockfd, char* cmds[], int numCmds)
{
	if(numCmds == 3) {
		setenv(cmds[1], cmds[2], ENV_OVERWR);
		return;
	} else {
		char *response = "Usage : setenv <ENV_name> <ENV_value>";
		write(clsockfd, response, strlen(response));
	}
	return;
}


void execChild(int clsockfd, char* cmds[], int* pipefd, int round, int numPartition)
{
	if (pipeNumberTable[0] && round == 0) { //rcv previous pipe
		puts("rcv previous pipe");
		close(unread);
		pipeNumber(0, 0, PIPENORE);
	}

	if (round != (numPartition - 1)) { //pipe to next cmd
		puts("pipe to next");
		pipeDup(pipefd, PIPE2NEXT);
	} else { //the last round or there is only one round
		puts("last cmd");
		if (isNumber(cmds[0])) { //pipe number
			puts("pipenumber");
			exit(EXIT_SUCCESS);
		} else if (isoutputFile(cmds)) { //output file
			outputfileHelper(clsockfd, pipefd, cmds);
			exit(EXIT_SUCCESS);  //no need to exec
		} else { //last cmd
			if (round != 0)
				pipeDup(pipefd, PIPE2IN);
			if(dup2(clsockfd, STDOUT_FILENO) == -1 || dup2(clsockfd, STDERR_FILENO) == -1)
				puts("pipeDup PIPE2CL");
		}
	}
	execCmds(clsockfd, cmds);
}


int* execParent(int* pipefd,char* cmds[], int round, int numPartition)
{
	if (round == (numPartition - 1) && isNumber(cmds[0])) { //pipe number
		pipeNumber(unread, atoi(cmds[0]), PIPENOWR);
	} else {
		if (unread != STDIN_FILENO)
			close(unread);
	}

	close(pipefd[1]);

	int childStatus;
	wait(&childStatus);
	numChild--;

	if (pipeNumberTable[0] && round == 0) { //close pipeNumberTable[0]
		pipeNumber(0, 0, PIPENOCL);
	}

	
	if (round == (numPartition - 1)) { //there is no remain cmds
		close(pipefd[0]);
		return NULL;
	}
	

	if (childStatus)  //childStatus != 0 -> unknown cmd
		return NULL;

	unread = pipefd[0];

	return newPipe(pipefd);
}


int isNumber(char* buffer)
{
	int i;

	for (i = 0; buffer[i] != '\0'; i++) {
		if (!isdigit(buffer[i]))
			return 0;
	}
	return 1;
}


int isoutputFile(char* cmds[])
{
	int i = 0;
	while (cmds[i] != NULL) {
		if (strcmp(cmds[i], ">") == 0)
			return 1;

		i++;
	}
	return 0;
}


void execCmds(int clsockfd, char* cmds[])
{
	if(execvp(cmds[0], cmds) == -1) {
		char *response;
		response = malloc(sizeof(char) * (BUFFSIZE + 1));
		sprintf(response, "Unknown command: [%s].\n", cmds[0]);
		write(clsockfd, response, strlen(response));  // return unknown msg to the client
	}
	exit(EXIT_FAILURE);
}


void outputfileHelper(int clsockfd, int* pipefd, char* cmds[])
{
	int i = 0;
	char** buffer;

	buffer = malloc(sizeof(char*));

	while(strcmp(cmds[i], ">") != 0) {
		buffer[i] = malloc(sizeof(char) * (strlen(cmds[i]) + 1));
		buffer[i] = strdup(cmds[i]);
		i++;
	}

	int pid = fork();
	numChild++;

	if (pid < 0)
		puts("output file fork failed");
	else if (pid == 0) {//child
		pipeDup(pipefd, PIPE2NEXT);
		execCmds(clsockfd, buffer); //exec cmds
	} else {// parent
		free(buffer);
		close(pipefd[1]);

		if (unread != STDIN_FILENO)
			close(unread);

		int childStatus;
		wait(&childStatus);
		numChild--;

		if(childStatus)
			exit(EXIT_FAILURE);

		char* parentbuffer = malloc(sizeof(char) * (BUFFSIZE + 1));
		read(pipefd[0], parentbuffer, (sizeof(char) * (BUFFSIZE + 1)));
		close(pipefd[0]);

		FILE *fPtr;
		fPtr = fopen(cmds[i + 1], "w");
		fprintf(fPtr, "%s", parentbuffer);
		fclose(fPtr);
	}
}


int parseInput(char* buffer, char* cmds[], char* delimt)
{
	if(!buffer)
		return 0;

	char* substr = NULL;
	char* saveptr = NULL;
	char* dupstr = strdup(buffer);

	substr = strtok_r(dupstr, delimt, &saveptr);
	cmds[0] = substr;

	if (substr == NULL) {
		puts("Error : client input\n");
		return -1;
	}

	int count = 1;  //count 0 is done
	while (substr) {
		substr = strtok_r(NULL, delimt, &saveptr);
		cmds[count] = substr;
		count++;
	}

	cmds[count] = NULL;
	return --count;
}
