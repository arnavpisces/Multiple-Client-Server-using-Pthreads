#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

/*
RED: 1
GREEN: 2
YELLOW: 3
BLUE: 4
MAGENTA:5
CYAN:6
*/

#define SERVER_PORT 8080

#define TRUE 1
#define FALSE 0

struct Clients{
	int userid;
}clients[15]; //10 client limit for the time being

int main(int argc, char* argv[]){
	int global=1;
	int len, rc, on=1, aux;
	int listen_sd=-1, new_sd=-1;
	int desc_ready, end_server=FALSE, compress_array=FALSE;
	int close_conn;
	char buffer[1024]={0};
	struct sockaddr_in6 addr;
	int timeout;
	struct pollfd fds[200];
	int nfds=1, current_size=0, i, j;
	// Clients clients[15];
	// memset(clients,0,sizeof(clients));
	int cli;
	for(cli=0;cli<15;cli++){
		clients[cli].userid=0;
	}

	listen_sd=socket(AF_INET6, SOCK_STREAM, 0);
	if(listen_sd<0){
		perror("Socket() failed");
		exit(-1);
	}

	rc=setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(rc<0){
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	rc=ioctl(listen_sd,FIONBIO, (char*)&on);
	if(rc<0){
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family=AF_INET6;
	memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
	addr.sin6_port=htons(SERVER_PORT);
	rc=bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));

	if(rc<0){
		perror("bind() failed");
		close(listen_sd);
		exit(-1);
	}

	rc=listen(listen_sd,32);
	if(rc<0){
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	memset(fds,0,sizeof(fds));

	fds[0].fd=listen_sd;
	fds[0].events=POLLIN;

	timeout=(4*60*1000); //timeout

	do{
			printf("Listening\n");
			rc=poll(fds,nfds,timeout);
			if(rc<0){
				perror("poll() failed");
				break;
			}
			if(rc==0){ //if the timeout (4 minutes) happened
				printf("poll() timed out. End program.\n");
				break;
			}


			current_size=nfds;
			for(i=0;i<current_size;i++){

				if(fds[i].revents==0){
					printf("yolo\n");
					continue;
				}


					if(fds[i].revents!=POLLIN){
						printf("Error! revents= %d\n",fds[i].revents);
						end_server=TRUE;
						break;
					}
					if(fds[i].fd==listen_sd){
						printf("Listening socket is readable\n");
						do{
							new_sd=accept(listen_sd,NULL,NULL);
							if(new_sd<0){
								if(errno!=EWOULDBLOCK){
									perror("accept() failed");
									end_server=TRUE;
								}
								break;
							}
							printf("New Incoming connection - %d\n", new_sd);
							fds[nfds].fd=new_sd;
							printf("NEW Descriptor%d\n",new_sd );
							fds[nfds].events=POLLIN;
							clients[new_sd].userid=new_sd;
							char newmsg[1024];
							sprintf(newmsg,"WELCOME, your user id is %d.\nTo broadcast message to everyone simply type the message and press enter.\nThe connected members are:\n",new_sd);
							char welcome[1024];
							sprintf(welcome,"User %d has joined the chat\n",new_sd);
							for(aux=0;aux<16;aux++){
								if(fds[aux].fd>0 && fds[aux].fd!=new_sd){
									send(fds[aux].fd,welcome,strlen(welcome),0);
								}
								char auxmsg[1024];
								if(clients[aux].userid!=0){
									sprintf(auxmsg,"USER: %d\n",clients[aux].userid);
									strcat(newmsg,auxmsg);
								}
							}
							if(send(fds[nfds].fd,newmsg,strlen(newmsg),0)<0){
								perror("send() failed");
							}
							global++; //user id incrementation
							nfds++;
						}while(new_sd!=-1);
					}
					else{
						printf("Descriptor %d is readable\n", fds[i].fd);
						close_conn=FALSE;

						do{
							rc=recv(fds[i].fd,buffer,sizeof(buffer),0);

							if(rc<0){
								if(errno!=EWOULDBLOCK){
									perror("recv() failed");
									close_conn=TRUE;
								}
								break;
							}
							if(rc==0){ //COPY THIS TO DOWN TO CHECK AND FULFILl THE CONNECTIONS CLOSED CONDITION
								printf("Connection closed\n");
								close_conn=TRUE;
								break;
							}
							len=rc;
							printf(" %d bytes reecived\n", len);

							// int cons;
							// for(cons=0;cons<current_size;cons++){
							// 	// if(fds[cons].fd>1){
							// 	printf("%d\n",fds[cons].fd);
							// 	printf("current_size %d\n",current_size);
							// 	break;
							// 		// rc=send(fds[cons].fd,buffer,len,0);
							// 		// if(rc<0){
							// 		// 	perror("send() failed");
							// 		// 	close_conn=TRUE;
							// 		// 	break;
							// 		// }
							// 	// }
							// }
							int abc;
							printf("%s\n",buffer);
							printf("buffer length %d\n",strlen(buffer));

							if(buffer[0]=='$' && buffer[1]=='M'){
								char idmsg[1024];
								sprintf(idmsg,"The Members connected are:\n");
								for(aux=0;aux<16;aux++){
									char auxmsg[1024];
									if(clients[aux].userid!=0){
										sprintf(auxmsg,"USER: %d\n",clients[aux].userid);
										strcat(idmsg,auxmsg);
									}
								}
								send(fds[i].fd,idmsg,strlen(idmsg),0);
								break;
							}

							int x;
							char msg[1024];
							sprintf(msg, ">>> %s\n",buffer);
							int bix=0;
							int okay=0;
							if(buffer[1]==':'){ //for handling c2c messages
								if(bix==0){
									bix==1;
								int yo;
								int hex=buffer[0]-'0';
								for(yo=0;yo<16;yo++){
									if(fds[yo].fd==hex)
									okay=1;
								}
								if(okay==0){
									char buf[]="No such user\n";
									send(fds[i].fd,buf,strlen(buf),0);
									break;
								}

							}
								buffer[1]='!';
								char actual[1024];
								int actp,k;
								for(actp=2,k=0;actp<strlen(buffer);actp++,k++){
									actual[k]=buffer[actp];
								}
								actual[k--]='\0';
								for(x=0;x<15;x++){
									printf("struct ids %d %d\n",clients[x].userid,fds[x].fd);
									int y=buffer[0]-'0';
									// printf("%d %d\n",y,clients[x].userid );
									char actmsg[1024]={0};
									sprintf(actmsg, ">>>(FROM %d) %s\n",fds[i].fd,actual);

									if(fds[x].fd=y){
										if(send(fds[x].fd,actmsg,strlen(actmsg),0)<0){
											perror("send() failed");
											bix=1;
											break;

										}

										break;

									}

								}

								break;

							}

							for(x=1;x<16;x++){
								printf("Struct userid %d\n",clients[fds[x].fd].userid);
								sprintf(msg, ">>>(Everyone by %d) %s\n",fds[i].fd,buffer);
								if(send(fds[x].fd,msg,strlen(msg),0)<0){
									perror("send() failed");

								}
								// else{
								// 	printf("Struct userid %d\n",clients[fds[x].fd].userid);
								// }
								printf("FD is %d\n",fds[x].fd);
							// 	// printf("%d\n",abc);
								// if(abc<0){
									// perror("send() failed");
									// close_conn=TRUE;
								// 	// break;
								// }
							}

							// for(x=0;x<16;x++){
							// 	if(clients[x].userid!=0){
							// 		if(send(fds[clients[x].userid].fd,msg,len+4,0)<0){
							// 			perror("send() failed");
							// 		}
							// 	}
							// }


							if(abc<0){
								perror("send() failed");
								close_conn=TRUE;
								break;
							}
							break;
						}while(TRUE);



						if(close_conn){
							close(fds[i].fd);
							fds[i].fd=-1;
							compress_array=TRUE;
						}

					}
			}
			if(compress_array){
				compress_array=FALSE;
				for(i=0;i<nfds;i++){
					if(fds[i].fd==-1){
						for(j=i;j<nfds;j++){
							fds[j].fd=fds[j+1].fd;
						}
						i--;
						nfds--;
					}
				}
			}
	}while(end_server==FALSE);

	for(i=0;i<nfds;i++){
		if(fds[i].fd>=0)
		close(fds[i].fd);
	}

}
