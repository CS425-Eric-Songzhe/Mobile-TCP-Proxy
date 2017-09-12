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
 * prepend length of payload to payload. this is the message to be sent
 */
void make_msg(char *msg, long len, char *input)
{

       long n = len;
       msg[0] = (n >> 24) & 0xFF;
       msg[1] = (n >> 16) & 0xFF;
       msg[2] = (n >> 8) & 0xFF;
       msg[3] = n & 0xFF;
       strncpy(&msg[4], input, len);
       //printf("%d,%d:|%s|", (int)len, (int)strlen(msg), msg);

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
	while(1){

	  // TODO: read input from stdin
	  char *input = "hello";

	  // make and send message 
	  long len = strlen(input); // number of bytes in payload
	  char msg[4+len];
	  make_msg(msg, len, input);
	  send(sock, msg, 4+len, 0);

	  break; // TODO: remove when looping
	}
        
	close(sock);    	
	return 0;
}
