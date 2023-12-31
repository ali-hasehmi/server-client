// includes for basic I/O and Other operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
//includes for socket programming
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

long fsize(char file_name[]){
		FILE *fp = fopen(file_name,"r");
		if(fp == NULL){
				perror("fsize() - fopen");
				return -1; 
		}
		fseek(fp,0L,SEEK_END);
		long file_size = ftell(fp);
		if(file_size == -1){
				perror("fsize() - ftell");
				fclose(fp);
				return -1;
		}
		fclose(fp);
		return file_size;
}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
		if(bytesleft != n){
				printf("need Resending Data - expected = %d ,send = %d",bytesleft,n);
		}
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

int GetString(char *buff,size_t size){
		int index = 0;
		char input;
		while(((input = getchar()) != '\n') && input != '\r'){
				buff[index++]= input;
				if(index+1 == size){
						buff[index] = '\0';
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

		char file_path[30];
		printf("Path To the File: ");
		scanf("%s",file_path);
		//GetString(file_path,30);
		long file_size = fsize(file_path);
		char msg[100];
		sprintf(msg,"transfer-file --name %s --size %ld",file_path,file_size);
		printf("file_path = %s\nfile_size=%ld\n",file_path,file_size);
		send(client_socket,msg,strlen(msg),0);
		long remaining = file_size;
		FILE *fp = fopen(file_path,"r");
		if(fp==NULL){
				perror("fopen"); 
				exit(1);
		}
		time_t before = time(NULL);
		int total_send=0;
		while(remaining){
			int sending_size;
			if(remaining >= 1024){
					sending_size = 1024;
					remaining -= 1024;
			}
			else{
					sending_size = remaining;
					remaining = 0; 
			}
			if (fread(buff,sending_size,1,fp)!=1){
					printf("Not haveing equal sizes in fread!\n");
			}
			if(sendall(client_socket,buff,&sending_size)){
					printf("Problem in sending Data!\n");
			}
			total_send += sending_size;
			print_bar(file_path,total_send,file_size);
			//printf("%d bytes sent - total sent: %d\n",sending_size,total_send);
			
		}
		time_t after = time(NULL);
		shutdown(client_socket,SHUT_WR);
		printf("done in %ldsec\n",after-before);
		printf("Press ENTER to continue...\n");
		getchar();
		getchar();
		fclose(fp);

		return EXIT_SUCCESS;
}
