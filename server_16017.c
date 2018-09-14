//Arnav Kumar - 2016017
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define SERVER_PORT 8080

#define TRUE 1
#define FALSE 0

char cmsg[5000];
int clients=0;

int sockfds[100];

void * sockThread(int sockaccept){
    int i=0;
    int id=++clients;
    int sock=sockaccept;
    char msg[5000];
    snprintf(msg,sizeof(msg),"Welcome Client Number %d\n",id);
    send(sock, msg,strlen(msg),0);
    while(1){
        memset(cmsg,'\0',sizeof(cmsg));
        recv(sock,cmsg,5000,0);
        if(strlen(cmsg)==0){
            printf("CLIENT %d LEFT THE SERVER\n",id);
            close(sock);
            sockfds[sock]=-1;
            pthread_exit(NULL);
        }
        char format[5000];
        snprintf(format,sizeof(format),"%d: %s",id,cmsg);
        for(i=0;i<100;i++){
            if(sockfds[i]!=-1){                
                send(sockfds[i],format,strlen(format),0);
            }
        }
        
        // printf("%s ----  YAH YAH YAH %d\n",cmsg,strlen(cmsg));
        if(!strcmp(cmsg,"CLOSE\n\0")){
            char closemsg[1024];
            strcpy(closemsg,"server says - CLOSING CONNECTION WITH YOU\n");
            send(sock,closemsg,strlen(closemsg),0);
            printf("CLOSING CONNECTION WITH Client number %d\n",id);
            close(sock);
            sockfds[sock]=-1;
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}

void * writer(int nouse){
    char msg[1000];
    int i=0;
    fgets(msg, sizeof(msg),stdin);
    // printf("sdhfkjasdhfjksahdjfk");
    char* shut;
    shut="SERVER is going to shutdown now\n";
    if(!strcmp(msg,"SHUTDOWN\n\0")){
        for(i=0;i<100;i++){
            if(sockfds[i]!=-1){
                send(sockfds[i],shut,strlen(shut),0);                
            }
        }
        // delay(300);
        exit(-1);
    }
}
int main(int argc, char const *argv[])
{
    int count=0;
    int i=0;
    for (i=0;i<100;i++){
        sockfds[i]=-1;
    }
    struct sockaddr_in addr={0}, cli;
    int listensd=-1;
    int rc=0, on=1;
    listensd=socket(AF_INET, SOCK_STREAM,0);
    if(listensd<0){
        perror("Socket() creation failed");
        exit(-1);
    }

    // rc=setsockopt(listensd,SOL_SOCKET,SO_REUSEADDR,(char *)&on, sizeof(on));
    // if(rc<0){
    //     perror("setsockopt() failed");
    //     close(listensd);
    //     exit(-1);
    // }
    // memset(&addr,0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    // memcpy(&addr.sin_addr, INADDR_ANY, sizeof(INADDR_ANY));
    addr.sin_port=htons(SERVER_PORT); //converts little-endian to big-endian format, host to network short
    rc=bind(listensd,(struct sockaddr *)&addr, sizeof(addr));
    
    if(rc<0){
        perror("bind() failed");
        close(listensd);
        exit(-1);
    }

    rc=listen(listensd,64);
    printf("Listening\n");
    if(rc<0){
		perror("listen() failed");
		close(listensd);
		exit(-1);
	}
    int sockaccept=0;
    pthread_t threads[100];
    pthread_t write;
    int tnum=0;
    int blah;
    blah=pthread_create(&write,NULL,writer,0);
    while(1){
        int clisize=sizeof(cli);
        sockaccept=accept(listensd,(struct sockaddr *)&cli, &clisize);
        
        if(sockaccept==-1){
            perror("Failed To Accept Connection\n");
            close(listensd);
            exit(-1);
        }
        sockfds[sockaccept]=sockaccept;
        int create=-1;
        create=pthread_create(&threads[tnum++],NULL, sockThread, sockaccept);
        if(create==-1){
            perror("Failed To Create Thread\n");
            close(listensd);
            exit(-1);
        }
        // tnum++;
    }
        pthread_join(threads[tnum++],NULL);

    return 0;
}
