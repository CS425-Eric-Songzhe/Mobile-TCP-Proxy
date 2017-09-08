//Description: Client side of the Moblie TCP Proxy project
//Author: Eric Evans, Songzhe Zhu
//Date: Sep 7 2017
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
  
int main(int argc, char const *argv[])
{
	//struct sockaddr_in address;
    	int sock = 0; //valread;
    	struct sockaddr_in serv_addr;
    	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("\n Socket creation error \n");
        	return -1;
    	}
  
    	memset(&serv_addr, '0', sizeof(serv_addr));
  
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);      		
  
    	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    	{	
        	printf("\nConnection Failed \n");
        	return -1;
    	}
	printf("Successfully connected to server\n");
    	send(sock , "hello\n" , strlen("hello") , 0 );
	//printf("check");
	//close(sock);
    	return 0;
}
