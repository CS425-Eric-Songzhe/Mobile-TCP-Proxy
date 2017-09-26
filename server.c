//Description: Server side of the Moblie TCP Proxy project
//Author: Eric Evans, Songzhe Zhu
//Date: Sep 7 2017
#include <sys/select.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h>
#include <sys/select.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <errno.h>
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
	//	  serv_addr->sin_addr.s_addr = inet_addr(ip);         
	memset(serv_addr, '0', sizeof(*serv_addr));
	serv_addr->sin_addr.s_addr = inet_addr(ip);
	//printf("ip NULL\n");
  }
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
  serv_addr->sin_port = htons(port);
	
  //memset(serv_addr, '0', sizeof(*serv_addr));         
  //serv_addr->sin_family = AF_INET;         
  //serv_addr->sin_port = htons(port);         
  //serv_addr->sin_addr.s_addr = inet_addr(ip);	


  return sock;
}


/*
 * bind port to server_fd socket and setup listening queue
 */  
void bind_and_listen(int server_fd, struct sockaddr_in *address, int backlog)
{
  if(bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)
	{
	  printf("BIND ERROR: %s\n", strerror(errno));
	  perror("bind failed");
	  exit(EXIT_FAILURE);
	}
  if (listen(server_fd, backlog) < 0)
	{
	  printf("Listen ERROR: %s\n", strerror(errno));
	  perror("listen");
	  exit(EXIT_FAILURE);
	}

}

/*
 * connect to server with serv_addr
 */
void connect_to_telnet(struct sockaddr_in *serv_addr, int sock)
{

  if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
	{
	  perror("Connection Failed");
	  exit(EXIT_FAILURE);
	}
  printf("telnet connected at: %d\n", sock);
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
 * return the first 4 bytes of buffer as an int
 */  
int get_length(char *buffer){

  int num = 0;

  num = buffer[0] << 24 | 
    (buffer[1] & 0xff) << 16 | 
    (buffer[2] & 0xff) << 8 | 
    (buffer[3] & 0xff);

  return num;
}


/*
 * MAIN
 */
int main(int argc, char const *argv[])
{
  int s1=0, s2=0, n=0, rv=0;
  fd_set readfds;
  struct timeval tv;
  char cmd_buf[1025], reply_buf[1025];
  // Read Arguments
  int port = atoi(argv[1]);

  // Setup Sockets
  printf("Setting up socket for cproxy\n");
  struct sockaddr_in address;
  int server_fd = setup_socket(&address, NULL, port);
  printf("- socket for cproxy open\n");

  printf("Binding to port for cproxy.\n");
  bind_and_listen(server_fd, &address, 5);
  printf("- Bind to port for cproxy successed\n");

  printf("Setting up socket for telnet (daemon)\n");
  struct sockaddr_in daemon_address;
  int server_teldaemon = setup_socket(&daemon_address, "127.0.0.1", 23);
  printf("- socket for telnet (daemon) open\n");
  // Connect with Telnet (Daemon)
  //connect_to_telnet(&daemon_address, server_teldaemon);

  // Forcefully attaching socket to port
  //	bind_and_listen(server_fd, &address, 5);
  connect_to_telnet(&daemon_address, server_teldaemon);
  //bind_and_listen(server_teldaemon, &daemon_address, 1);	

  // Client Loop	
  int new_socket = 0;
  int len = 0;
  char buffer[1024] = {0};
  while(1){	
	// Accept client
	printf("Accepting from cProxy\n");
	s1 = accept_client(server_fd, &address);
	s2 = server_teldaemon;
	printf("- Accepted\n");

	//printf("Accepting from daemon\n");
	//s2 = accept_client(server_teldaemon, &daemon_address);
	//printf("- Accepted\n");
	//printf("A client connected!\n");
	while(1){
	  // Receive messages from new_socket
	  //while((len=recv(s1, buffer, sizeof(buffer), 0)) > 0){
	  //printf("%s\n",buffer);	   
	  FD_ZERO(&readfds);                  
	  // add our descriptors to the set
	  FD_SET(s1, &readfds);
	  FD_SET(s2, &readfds);
	  // find the largest descriptor, and plus one.
	  if (s1 > s2) n = s1 + 1;                 
	  else n = s2 +1;

	  // wait until either socket has data ready to be recv()d (timeout 10.5 secs)                 
	  tv.tv_sec = 10;
	  tv.tv_usec = 500000;                 
	  rv = select(n, &readfds, NULL, NULL, &tv);
 	    

	  if (rv == -1) {                         
		perror("select"); // error occurred in select()
	  } else if (rv == 0) {
		printf("Timeout occurred!  No data after 10.5 seconds.\n");
	  }else {
		// one or both of the descriptors have data
		if (FD_ISSET(s1, &readfds)) {
		  recv(s1, cmd_buf, sizeof(cmd_buf), 0);
		  printf("Recved command from cproxy: %s\n", cmd_buf);
		  send(s2, cmd_buf, strlen(cmd_buf), 0);
		  memset(cmd_buf, 0, sizeof(cmd_buf));
		}
		if (FD_ISSET(s2, &readfds)) {
		  recv(s2, reply_buf, sizeof(reply_buf), 0);
		  printf("Recved reply from daemon: %s\n", reply_buf);
		  //TODO: Send to cproxy
		  send(s1, reply_buf, strlen(reply_buf), 0);
		  memset(reply_buf, 0, sizeof(reply_buf));
		}
      }

	  // get and print payload length
	  //printf("Received message from client: \n");
	  //int paylen = get_length(buffer);
	  //printf("%d\n", paylen);
		       
	  // get and print payload -- start 4 bytes in
	  //int i=0;
	  //for(i=4; i < 4+paylen; i++)
	  //printf("%c",buffer[i]);
	  //printf("\n\n");
		       
	}
	if(len == 0)
	  printf("Nothing more from the client. Client has been shut down.\n");
	if(len < 0)
	  printf("ERROR on RECV()!\n");
	
	close(s1);
	close(s2);
  }
	
  close(server_fd);
  //	close(server_teldaemon);
  return 0;
}
