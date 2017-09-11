//Description: Server side of the Moblie TCP Proxy project
//Author: Eric Evans, Songzhe Zhu
//Date: Sep 7 2017
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <netinet/in.h> 
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
	

	if (ip != NULL){
	  serv_addr->sin_addr.s_addr = inet_addr(ip);         
	  memset(serv_addr, '0', sizeof(*serv_addr));
	}
    	serv_addr->sin_family = AF_INET;
    	serv_addr->sin_port = htons(port);

	return sock;
}


/*
 * bind port to server_fd socket and setup listening queue
 */  
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
 * MAIN
 */
int main(int argc, char const *argv[])
{
        // Read Arguments
        int port = atoi(argv[1]);

	// Setup Socket
    	struct sockaddr_in address;
	int server_fd = setup_socket(&address, NULL, port);
 
    	// Forcefully attaching socket to port
	bind_and_listen(server_fd, &address, 5);
	
	// Client Loop	
	int new_socket = 0;
	int len = 0;
    	char buffer[1024] = {0};
	while(1){
	        // Accept new client
	        new_socket = accept_client(server_fd, &address);
		printf("A client connected!\n");

		// Receive messages
		while((len=recv(new_socket, buffer, sizeof(buffer), 0)) > 0){
			//TODO: read and print
			printf("Client: %s\n", buffer);
		}
		if(len == 0){
			printf("Nothing more from the client. Client has been shut down.\n");
		}
		if(len < 0){
			printf("ERROR on RECV()!\n");
		}
		//printf("%s", buffer);
		close(new_socket);
    	}

	close(server_fd);
    	return 0;
}
