// includes for basic operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
// includes for socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int print_bar(char *msg,long double done, long double total) {
			if(msg == NULL) {
					msg = "";
			}
			long double percentage = done/total * 100;
			int completed_num = percentage * 30 / 100 ; // on scale of 30
			printf("\r%s[",msg);
			for(int i = 0 ; i < 30 ; ++i) {
					if(completed_num){
							putchar('#');
							completed_num--;
					}
					else{
							putchar('.');
					}
			}
			printf("] %.2Lf%%",percentage);
			if(percentage == 100.0 ) {
				printf("\n");
			}
			fflush(stdout);
	}

int main(int argc, char *argv[]) {

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
		status = recv(new_socket,buffer,len,0);
		buffer[status] = '\0';
		char file_path[30];
		long file_size;
		printf("msg = %s\n",buffer);
		sscanf(buffer,"transfer-file --name %s --size %ld",file_path,&file_size);
		char new_file[45] = "server-";
		strcat(new_file,file_path);
		FILE *fp = fopen(new_file, "w");		
		long total= file_size,
			 expected_recv,
			 real_recv,
			 total_recv = 0;
			 time_t before = time(NULL);
		while(total>0){
				if(total>= 1024){
						expected_recv= 1024;
				}
				else{
						expected_recv = total;
				}
				if((real_recv=recv(new_socket,buffer,1024,0)) != expected_recv){
						printf("Not having equal sizes in recv!\n");
				}
				total_recv += real_recv;
				if(real_recv == 0 ){
						printf("disconnect from client!\n");
						break;
				}

				print_bar(new_file,total_recv,file_size);
			//	printf("%ld bytes recieved - total = %ld\n",real_recv,total_recv);
				total -= real_recv;
				if(fwrite(buffer,real_recv,1,fp) != 1){
						printf("Not having equal sizes in fwrite!\n");
				}
		}
		
		fclose(fp);
		
		time_t after = time(NULL);
		printf("recieved in %ldsec\n",after-before);
		close(new_socket);
		close(sfd);
		return EXIT_SUCCESS;
		}

