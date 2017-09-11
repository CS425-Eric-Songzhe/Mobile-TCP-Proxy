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


/*
 * set up socket and set serv_addr fields 
 */  
int setup_socket(struct sockaddr_in *serv_addr, char const *ip, int port)
{
        int sock = 0;
    	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
    	}
	
    	memset(serv_addr, '0', sizeof(*serv_addr));
    	serv_addr->sin_family = AF_INET;
    	serv_addr->sin_port = htons(port);
	serv_addr->sin_addr.s_addr = inet_addr(ip);         

	return sock;
}


/*
 * connect to server with serv_addr
 */
void connect_to_server(struct sockaddr_in *serv_addr, int sock)
{

    	if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
    	{	
        	printf("\nConnection Failed \n");
                exit(EXIT_FAILURE);
    	}

}


/*
 * MAIN
 */
int main(int argc, char const *argv[])
{
        // Read Arguments
	char const *ip = argv[1];
	int port = atoi(argv[2]);

        // Setup Socket 
    	struct sockaddr_in serv_addr;
	int sock = setup_socket(&serv_addr, ip, port);

	// Connect to Server
	connect_to_server(&serv_addr, sock);
	printf("Successfully connected to server\n");
    	
	// Read and Send Messages
	send(sock , "hello\n" , strlen("hello") , 0 );
	
	//printf("check");
	
	close(sock);    	
	return 0;
}
