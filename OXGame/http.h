#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#define SA struct sockaddr
#define MAXCLI 100
#define BUFSIZE 4096

typedef struct chessboard{
	char bd[9];
}chessboard;

typedef struct PlayerInfo{
	unsigned int connfd;
	char *name;
	char ini , id , inuse , online , ingame , query , turn;
	chessboard *board;
}PlayerInfo;

void ShowBoard(PlayerInfo *);
char checkBoard(PlayerInfo *);
void BattleQuery(PlayerInfo *);
void Battle(PlayerInfo *);
void Logout(PlayerInfo *);
void ShowOnlinePlayer(unsigned int);
void menu(unsigned int);
void startSvr(const char *);
void Service(void *);
char Login(int);
int response(char , char *);

