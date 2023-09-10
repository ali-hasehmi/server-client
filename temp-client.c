// includes for basic I/O and Other operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//includes for socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
int GetString(char *buff,size_t size){
		int index = 0;
		char input;
		while((input = getchar()) != '\n' && input != '\r'){
				buff[index++]= input ;
				if(index == size){
						return 1;
				}
						
		}
		buff[index] = '\0';
		return 0;
}
int main(int argc, char *argv[]) {
		// get ip and port from user
		char ip_addr[15];
		char port_num[7];
		printf("IP Address: ");
		scanf("%s",ip_addr);
		printf("Port: ");
		scanf("%s",port_num);
		
		int status,
			client_socket;

		struct addrinfo hints,
						*res = NULL, // will point to the linked-list
						*next;

		memset(&hints,0,sizeof(hints)); // make sure the struct is empty
		hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP stream socket
	
		if( (status = getaddrinfo(ip_addr, port_num, &hints,&res)) != 0 ) {
				fprintf(stderr,"getaddrinfo error: %s\n",gai_strerror(status));
				exit(1);
		}	
		// res now points to a linked-list of 1 more struct addrinfo
		// don't forget to free up the linked-list
		client_socket = -1;
		status = -1;
		for( next = res; next != NULL ; next = next->ai_next ) {
				if((client_socket = socket( next->ai_family, next->ai_socktype,next->ai_protocol)) == -1){
						perror("socket");
				}
				else{
					if((status = connect(client_socket, next->ai_addr, next->ai_addrlen)) == -1){
							perror("connect");
					}
					else{
					break;
					}
				}
		}
		if(client_socket == -1 || status == -1) {
				fprintf(stderr,"NO Luck Today\n");
				exit(1);
		}
		printf("connected\n");
		char buff[1024];
		int recieved_bytes;
		recieved_bytes = recv(client_socket, buff, 1024, 0);
		buff[recieved_bytes] = '\0';
		printf("%s\n",buff);
		while(getchar() != 'q'){
			GetString(buff,1024);
			send(client_socket,buff,strlen(buff),0);			
		}

		return EXIT_SUCCESS;
}
