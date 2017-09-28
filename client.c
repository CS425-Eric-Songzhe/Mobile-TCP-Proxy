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
#include <sys/select.h>


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
  int s1=0, s2=0, n=0, rv=0;
  fd_set readfds;
  struct timeval tv;
  char cmd_buf[1025], reply_buf[1025];
	
  // Read Arguments
  char const *ip = argv[2]; // server ip address
  int port_sproxy = atoi(argv[3]); // server port
  int port_telnet = atoi(argv[1]); // telnet port

  // Create a buffer
  //char buffer[1025]; // extra char for '\n'

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
  printf("Sending connect request to sproxy\n");
  //connect_to_server(&serv_addr, sock);
  connect_to_server(&addr_telnet, sock_telnet);
  printf("- connected to sproxy\n");

  // Connect with Telnet
  /*printf("Listening for telnet connect request\n");
    bind_and_listen(sock_telnet, &addr_telnet, 5);
    printf("- connected to telnet\n");
  */
  //printf("Listening for daemon connect request\n");
  //sleep(10);
  //bind_and_listen(sock, &serv_addr, 5);
  //printf("- connected to daemon\n");

  printf("Listening for telnet connect request\n");
  bind_and_listen(sock, &serv_addr, 5);
  printf("- connected to telnet\n");
  // Read Incoming Data from Telent and Send to Sproxy
  int new_socket = 0;
  int len1 = 0, len2 = 0;
  while(1){
    // Accept telnet
    printf("Accepting request from telnet\n");
    s1 = accept_client(sock, &serv_addr);
    printf("- telnet request accepted\n");

    // printf("Accepting request from daemon\n");
    s2 = sock_telnet;
    //printf("- daemon request accepted\n");
	
    while(1){
      // clear the set ahead of time
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
		  len1 = recv(s1, cmd_buf, sizeof(cmd_buf), 0);
		  printf("Recved command from telnet: %s\n", cmd_buf);
		  send(s2, cmd_buf, len1, 0);
		  memset(cmd_buf, 0, sizeof(cmd_buf));
		}
		if (FD_ISSET(s2, &readfds)) {
		  len2 = recv(s2, reply_buf, sizeof(reply_buf), 0);
		  printf("Recved reply from server: %s\n", reply_buf);
	          send(s1, reply_buf, len2, 0);
		  memset(reply_buf, 0, sizeof(reply_buf));
		}
      }	
      // Receive messages from new_socket
      /*while((len=recv(new_socket, buffer, sizeof(buffer), 0)) > 0){
		printf("Received message from telnet \n");
		printf("%s",buffer);
		send(sock, buffer, strlen(buffer), 0);
		}
		if(len == 0)
		printf("Nothing more from telnet. Telnet has closed.\n");
		if(len < 0)
		printf("ERROR on RECV()!\n");*/
    }
    close(s1);
    close(s2);
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
	  
  //close(sock_telnet);    	
  return 0;
}
