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
        	perror("Socket creation error");
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
        	perror("Connection Failed");
		printf("%d\n", sock);
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

/*  * bind port to server_fd socket and setup listening queue  */ 
void bind_and_listen(int server_fd, struct sockaddr_in *address, int backlog) 
{         
	if(bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)         
	{                 
		perror("bind failed");                 
		exit(EXIT_FAILURE);         
	}         
	if (listen(server_fd, backlog) < 0)         
	{                 
		perror("listen");                 
		exit(EXIT_FAILURE);         
	}
	printf("bind&listen success on %d\n", server_fd);  
}



/*
 * replace the last character with '\0'
 */
void remove_last_char(char* buffer){

  int len = strlen(buffer);
  buffer[len-1] = '\0';
}



/*
 * MAIN
 */
int main(int argc, char const *argv[])
{
        // Read Arguments
	char const *ip = argv[2];
	int port_sproxy = atoi(argv[3]);
	int port_telnet = atoi(argv[1]);

        // Create a buffer
        char buffer[1025]; // extra char for '\n'

        // Setup Socket 
    	struct sockaddr_in serv_addr;
	int sock = setup_socket(&serv_addr, ip, port_sproxy);

	struct sockaddr_in serv_addr_telnet;
        int sock_telnet = setup_socket(&serv_addr_telnet, "127.0.0.1", port_telnet);

	// Connect to Server
	//connect_to_server(&serv_addr, sock);
	bind_and_listen(sock_telnet, &serv_addr_telnet, 5);
	//printf("Successfully connected to server\n");
    	
	// Read and Send Messages
	//printf("Please enter the message: \n"); 
	//read input from stdin
	while(fgets(buffer, sizeof(buffer), stdin) != NULL){
	  remove_last_char(buffer); // strips \n char
	    
	  // get length as 4-byte long
	  long len = strlen(buffer); // num bytes in payload
	  if(len == 0) // enures a blank message is not sent
	    continue; 

	  // make and send message 
	  char msg[4+len]; // make room for int (payload size)
	  make_msg(msg, len, buffer);
	  send(sock_telnet, msg, 4+len, 0);
	}
	//if(feof(stdin))
	//  printf("Communication Sucessfully Terminated\n");
	//else
	//  printf("ERROR: Communication Interrupted\n");
	  
	close(sock_telnet);    	
	return 0;
}
