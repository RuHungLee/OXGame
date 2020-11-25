#include    "http.h"

char LoginTable[MAXCLI];
PlayerInfo playerary[MAXCLI];

void startSvr(const char *port){

    int listenfd , connfd , cpid , option , i , j;
    struct sockaddr_in seraddr , cliaddr;
    socklen_t clilen;

    pthread_t * tid = (pthread_t*)malloc(sizeof(pthread_t)*MAXCLI);
    memset((void *)tid , 0 , sizeof(pthread_t)*MAXCLI);
	// signal(SIGCHLD,SIG_IGN);
	
    printf(
            "Server started %shttp://127.0.0.1:%s%s\n",
            "\033[92m",port,"\033[0m"
            );

	//create socket.
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) > 0){
        printf("The socket was created! , the listenfd is %d\n" , listenfd);
    }else{perror("Error occured when creating socket......\n");}

    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = INADDR_ANY;
    seraddr.sin_port = htons(atoi(port));
	
	//bind socket on specified port.
    if(bind(listenfd , (SA *)&seraddr , sizeof(seraddr))!=0 || listen(listenfd , 30)< 0){
        perror("Binding or Listening error......\n");
        exit(1);
	}

	//wait for syn packet and try to connect to client.
    for(;;){    

        printf("listening socket!\n");
		if((connfd = accept(listenfd , (SA *)&cliaddr , &clilen))<0){
            if(errno == EINTR){
                continue;
            }else{

                perror("accept error!");
            }
        }
        printf("connfd : %d\n", connfd);
        for(i = 0 ; i < 100 ; i++){
            if(tid[i] != NULL){
                continue;
            }else{
                

                for(j = 0 ; j < 100; j++){
                    if(playerary[j].inuse == 0){
                        // printf(" i %d j %d found!\n" , i , j);
                        playerary[j].id = j+1;
                        playerary[j].inuse = 1;
                        playerary[j].connfd = connfd;
                        pthread_create(&tid[i] , NULL , (void *)Service , (void *)&playerary[j]);
                        break;
                    }
                }

                if(j == 100){printf("client is too many!\n");}
                break;
                
            }
        }
	}
}

void checkBattleQueue(){
    
    char buf[512];
    for(int i = 0 ; i<100 ; i++){
        if(playerary[i].online && playerary[i].query && !playerary[i].ingame){
            dprintf(playerary[i].connfd , "\n玩家 %s 向你發起挑戰 , 是否接受挑戰(y/n) :" , playerary[playerary[i].query-1].name);   
        }
    }

    alarm(5);
}

void Service(void * info){

    unsigned int i , n , fd;
    char opt , first = 1;
    char user[512] , pwd[512] , ruser[512] , rpwd[512] , cmd[512];
    PlayerInfo * player = (PlayerInfo *)info;
    alarm(5);
    signal(SIGALRM , checkBattleQueue);
    fd = player->connfd;
    FILE *fp = fopen("./shadow" , "r");
    if(fp == NULL){printf("failed");}
    dprintf(fd , "\nOX GAME!\n\n");
    
    while(player->online == 0){

        if(!first){dprintf(fd , "帳號或密碼有誤 !\n");}
        dprintf(fd , "使用者名稱 : ");
        n = read(fd , user , 512);
        user[n-1] = 0;
        dprintf(fd , "密碼 : ");
        n = read(fd , pwd , 512);
        pwd[n-1] = 0;

        while(fscanf(fp , "%s %s" , ruser , rpwd)!=EOF){
            if(strcmp(user ,ruser) == 0){
                if(strcmp(pwd , rpwd) == 0){
                    player->online = 1;
                    player->name = user;
                    dprintf(fd , "登錄成功!\n");
                    break;
                }
            }  
        }

        fseek(fp , 0 , SEEK_SET);
        
    }

    for(;;){

        menu(fd);
        n = read(fd , cmd , 512);
        cmd[n-1] = 0;
        if(cmd[0] == '1'){
            ShowOnlinePlayer(fd);
        }else if(cmd[0] == '2'){
            BattleQuery(player);
        }else if(cmd[0] == '3'){
            Logout(player);
        }else if(cmd[0] == 'y'){
            if(player->query != 0){
                playerary[player->query-1].query = player->id;
                Battle(player);
            }
        }
        
    }    
}

void menu(unsigned int fd){
    dprintf(fd , "\n================================\n");
    dprintf(fd , "     1. 查看在線玩家\n");
    dprintf(fd , "     2. 向玩家發起挑戰\n");
    dprintf(fd , "     3. 登出遊戲\n");
    dprintf(fd , "================================\n");
    dprintf(fd , "command : ");
}


void ShowOnlinePlayer(unsigned int fd){
    dprintf(fd , "\n在線玩家列表\n");
    for(int i = 0 ; i<100 ; i++){
        if(playerary[i].online == 1){
            dprintf(fd , "玩家 ID : %s\n" , playerary[i].name);
        }
    }
}

void Logout(PlayerInfo *player){
    memset((void *)player , 0 , sizeof(player));
    exit(0);
}

void Battle(PlayerInfo *player){

    char n , buf[2];
    int opt;
    player->ingame = 1;
    if(player->ini == 1){
        player->turn = 1;
        player->board = (chessboard *)malloc(sizeof(chessboard));
        playerary[player->query-1].board = player->board;
        memset(player->board , ' ' , sizeof(chessboard));
    }else{
        while(1){if(player->board!=NULL){break;}}
    }


    while(1){
        
        ShowBoard(player);
        n = checkBoard(player);

        if(n == 1){

            player->ini == 1? (dprintf(player->connfd , "%s is Winner!\n" , player->name)): (dprintf(player->connfd , "%s is winner!\n" , playerary[player->query-1].name));
            break;
        }else if(n == 2){

            player->ini == 1? (dprintf(player->connfd , "%s is Winner!\n" , playerary[player->query-1].name)) : (dprintf(player->connfd , "%s is Winner!\n" , player->name));
            break;
        }else{

            if(player->turn == 1){
                dprintf(player->connfd , "請選擇旗位: ");
                read(player->connfd , buf , 2);
                buf[1] = 0;
                opt = atoi(buf);
                if(player->ini == 1){
                    player->board->bd[opt] = 'o';
                }else{
                    player->board->bd[opt] = 'x';
                }
                player->turn = 0;
                playerary[player->query-1].turn = 1;
                printf("test2\n");
            }else{
                while(1){
                    if(player->turn == 1){break;}
                }; 
            }
        }
    }

    if(player->ini == 1){
        player->query = 0;
        player->turn = 0;
        free(player->board);
        player->board = NULL;
    }else{
        while(1){
            if(playerary[player->query-1].board == NULL);
            player->query = 0;
            player->turn = 0;
            player->board = NULL;
            break;
        }
    }

}

char checkBoard(PlayerInfo *player){
    int i , j;
    chessboard *bd = player->board;

    if(bd->bd[0] == 'o' && bd->bd[1] == 'o' && bd->bd[2] == 'o'){return 1;}
    if(bd->bd[3] == 'o' && bd->bd[4] == 'o' && bd->bd[5] == 'o'){return 1;}
    if(bd->bd[6] == 'o' && bd->bd[7] == 'o' && bd->bd[8] == 'o'){return 1;}   
    if(bd->bd[0] == 'o' && bd->bd[3] == 'o' && bd->bd[6] == 'o'){return 1;}
    if(bd->bd[1] == 'o' && bd->bd[4] == 'o' && bd->bd[7] == 'o'){return 1;}
    if(bd->bd[2] == 'o' && bd->bd[5] == 'o' && bd->bd[8] == 'o'){return 1;}
    if(bd->bd[0] == 'o' && bd->bd[4] == 'o' && bd->bd[8] == 'o'){return 1;}        
    if(bd->bd[2] == 'o' && bd->bd[4] == 'o' && bd->bd[6] == 'o'){return 1;}

    if(bd->bd[0] == 'x' && bd->bd[1] == 'x' && bd->bd[2] == 'x'){return 2;}
    if(bd->bd[3] == 'x' && bd->bd[4] == 'x' && bd->bd[5] == 'x'){return 2;}
    if(bd->bd[6] == 'x' && bd->bd[7] == 'x' && bd->bd[8] == 'x'){return 2;}   
    if(bd->bd[0] == 'x' && bd->bd[3] == 'x' && bd->bd[6] == 'x'){return 2;}
    if(bd->bd[1] == 'x' && bd->bd[4] == 'x' && bd->bd[7] == 'x'){return 2;}
    if(bd->bd[2] == 'x' && bd->bd[5] == 'x' && bd->bd[8] == 'x'){return 2;}    
    if(bd->bd[0] == 'x' && bd->bd[4] == 'x' && bd->bd[8] == 'x'){return 2;}        
    if(bd->bd[2] == 'x' && bd->bd[4] == 'x' && bd->bd[6] == 'x'){return 2;}       

}

void ShowBoard(PlayerInfo *player){
    
    dprintf(player->connfd , "\n\t%c | %c | %c\n" , (player->board)->bd[0] , (player->board)->bd[1] , (player->board)->bd[2]);
    dprintf(player->connfd , "\t-----------\n");
    dprintf(player->connfd , "\t%c | %c | %c\n" , (player->board)->bd[3] , (player->board)->bd[4] , (player->board)->bd[5]);
    dprintf(player->connfd , "\t-----------\n");
    dprintf(player->connfd , "\t%c | %c | %c\n\n" , (player->board)->bd[6] , (player->board)->bd[7] , (player->board)->bd[8]);

}

void BattleQuery(PlayerInfo *player){

    unsigned int i , n;
    char name[512];
    dprintf(player->connfd , "\n請輸入挑戰玩家 ID : ");
    n = read(player->connfd , name , 512);
    name[n-1] = 0;

    for(i = 0 ; i<100 ; i++){
        if(playerary[i].online && strcmp(playerary[i].name , name) == 0){
            
            playerary[i].query = player->id;
            dprintf(player->connfd , "等待對方回應中......\n");
            
            while(1){
                
                if(player->query == playerary[i].id){
                    
                    printf("player->query : %d\nplayerary[i].id : %d\n" , player->query , playerary[i].id);
                    player->ini = 1;
                    Battle(player);
                    break;
                
                }
            
            }

            break;
        }
    }
}