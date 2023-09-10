// includes for basic operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
// includes for socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char *argv) {

		int status,
			sfd, // socket file descriptor
			new_socket;

		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof(their_addr);

		struct addrinfo hints,
						*servinfo = NULL, // will point to the result
						*next;

		memset(&hints,0,sizeof(hints)); // make sure the struct is empty
		hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
		hints.ai_flags = AI_PASSIVE; // fill in my IP for me

		if((status = getaddrinfo(NULL,"8585",&hints,&servinfo)) !=0) {
				fprintf(stderr,"getaddrinfo error: %s\n",gai_strerror(status));
				exit(1);
		}
		// servinfo now points to a linked list of 1 more struct addrinfo
		// when you don't need servinfo anymore, must free linked list
		for( next = servinfo ; next != NULL ; next = next->ai_next ) {
				if((sfd = socket( next->ai_family, next->ai_socktype, next->ai_protocol)) != -1 ) {
						if( (status = bind(sfd, next->ai_addr, next->ai_addrlen)) != -1) {
								break;
						}
						else{
								perror("bind");
						}
				}
				else{
						perror("socket");
				}
		}
		if(sfd == -1){ // if all the addrinfo-s failed
				exit(1);
		}

		freeaddrinfo(servinfo); // free the linked-list

		// listen to incoming connections
		if( (status = listen(sfd, 5)) == -1 ) {
				perror("listen");
		}
		
		if( (new_socket = accept(sfd, (struct sockaddr *)&their_addr, &addr_size)) == -1 ){
				perror("accept");
		} 
		char buffer[1024] = "The only thing people are truly equal in is just death\n";
		int len = strlen(buffer);
		
		if((status = send(new_socket, buffer, len, 0)) == -1){
				perror("send");
		}
		len = 1024;
		while( (status = recv(new_socket,buffer,len,0)) > 0) {
				buffer[status]='\0';
				printf("%s\n",buffer);
		}

		close(new_socket);
		close(sfd);
		return EXIT_SUCCESS;
		}

