//Description: Server side of the Moblie TCP Proxy project
//Author: Eric Evans, Songzhe Zhu
//Date: Sep 7 2017
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
  
int main(int argc, char const *argv[])
{
	int server_fd, new_socket;
    	struct sockaddr_in address;
	int len = 0;
    	int addrlen = sizeof(address);
    	char buffer[1024] = {0};
    //char *hello = "Hello from server";
      
    	// Creating socket file descriptor
    	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    	{
        	perror("socket failed");
        	exit(EXIT_FAILURE);
    	}
      
    	address.sin_family = AF_INET;
    	address.sin_addr.s_addr = INADDR_ANY;
    	address.sin_port = htons(atoi(argv[1]));
      
    	// Forcefully attaching socket to the port 8080
    	if(bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    	{
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}
    	if (listen(server_fd, 5) < 0)
    	{
        	perror("listen");
        	exit(EXIT_FAILURE);
    	}
	while(1){
    		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    		{
        		perror("accept");
        		exit(EXIT_FAILURE);
    		}
		else{
			printf("A client connected!\n");
		}
		//This is the size of the input, returns 5 for "hello"
		//len = recv(new_socket, buffer, sizeof(buffer), 0);
		//printf("%d", len);

		while(len=recv(new_socket, buffer, sizeof(buffer), 0)/*len*/>0){
			//TODO: read and print
			printf("Client: %s\n", buffer);
		}
		if(len==0){
			printf("Nothing more from the client. Client has been shut down.\n");
		}
		if(len<0){
			printf("ERROR on RECV()!\n");
		}
		//printf("%s", buffer);
		//close(new_socket);
    	}
    	return 0;
}
