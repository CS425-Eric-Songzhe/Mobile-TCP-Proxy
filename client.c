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
 * accept a new client and return socket value                                                                                          
 */
int accept_client(int server_fd, struct sockaddr_in *address)
{
  int sock = 0;
  int addrlen = sizeof(*address);
  if ((sock = accept(server_fd, (struct sockaddr *)address, (socklen_t*)&addrlen)) < 0)
    {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

  return sock;
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
	printf("(bind&listen success on %d)\n", server_fd);  
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
        char const *ip = argv[2]; // server ip address
	int port_sproxy = atoi(argv[3]); // server port
	int port_telnet = atoi(argv[1]); // telnet port

        // Create a buffer
        char buffer[1025]; // extra char for '\n'

        // Setup Sockets
	printf("Setting up socket for sproxy\n");
    	struct sockaddr_in serv_addr;
	int sock = setup_socket(&serv_addr, ip, port_sproxy);
	printf("- socket for sproxy open\n");

	printf("Setting up socket for telnet\n");
	struct sockaddr_in addr_telnet;
        int sock_telnet = setup_socket(&addr_telnet, "127.0.0.1", port_telnet);
	printf("- socket for telnet open\n");

	// Connect to Server
	//printf("Sending connect request to sproxy\n");
	//connect_to_server(&serv_addr, sock);
	//printf("- connected to sproxy\n");

	// Connect with Telnet
	printf("Listening for telnet connect request\n");
	bind_and_listen(sock_telnet, &addr_telnet, 5);
	printf("- connected to telnet\n");

	// Read Incoming Data from Telent and Send to Sproxy
	int new_socket = 0;
	int len = 0;
	while(1){
	  // Accept telnet
	  printf("Accepting request from telnet\n");
	  new_socket = accept_client(sock_telnet, &addr_telnet);
	  printf("- telnet request accepted\n");

	  // Receive messages from new_socket
	  while((len=recv(new_socket, buffer, sizeof(buffer), 0)) > 0){
	    printf("Received message from telnet \n");
	    printf("%s",buffer);

	  }
	  if(len == 0)
	    printf("Nothing more from telnet. Telnet has closed.\n");
	  if(len < 0)
	    printf("ERROR on RECV()!\n");

	  close(new_socket);
	}
	
	//read input from stdin
	//while(fgets(buffer, sizeof(buffer), stdin) != NULL){
	//  remove_last_char(buffer); // strips \n char
	    
	  // get length as 4-byte long
	// long len = strlen(buffer); // num bytes in payload
	//if(len == 0) // enures a blank message is not sent
	    //  continue; 

	  // make and send message 
	//char msg[4+len]; // make room for int (payload size)
	//make_msg(msg, len, buffer);
	//send(sock_telnet, msg, 4+len, 0);
	//}
	//if(feof(stdin))
	//  printf("Communication Sucessfully Terminated\n");
	//else
	//  printf("ERROR: Communication Interrupted\n");
	  
	close(sock_telnet);    	
	return 0;
}
