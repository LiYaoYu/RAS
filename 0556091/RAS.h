#ifndef _RAS_H
#define _RAS_H

#define ENVPATH_NAME "PATH"
#define ENVPATH_VALUE "bin:."
#define ENV_OVERWR 1

#define BUFFSIZE 15000
#define MAXCMDSIZE 256

extern char** environ;
extern int unread;

int numChild;

void changeFolder(char* folder);

void init_environ();

void welcomemsg(int clsockfd);

void clientHelper(int clsocdfd);

void get_and_execute_msg(int clsockfd);

void prompt(int clsocdfd);

void cmdHelper(int clsocdfd, char* clInput);

int isIllegal(char* clInput);

void exitHelper(int clsockfd);

void printenvHelper(int clsockfd, char* cmds[], int numCmds);

void setenvHelper(int clsockfd, char* cmds[], int numCmds);

void execChild(int clsockfd, char* cmds[], int* pipefd, int round, int numPartition);

int* execParent(int* pipefd,char* cmds[], int round,int numPartition);

int isNumber(char* cmds);

int isoutputFile(char* cmds[]);

void execCmds(int clsockfd, char* cmds[]);

void outputfileHelper(int clsockfd, int* pipefd, char* cmds[]);

int parseInput(char* buffer, char* cmds[], char* delimt);

#endif