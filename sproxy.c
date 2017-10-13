//Description: Server side of the Moblie TCP Proxy project
//Authors: Eric Evans, Songzhe Zhu
//Date: Sep 29 2017
#include <time.h>
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
#include "mysockets.h"
#include "mymessages.h"

/*
 * MAIN
 */
int main(int argc, char const *argv[])
{
    int s1 = 0, s2 = 0, n = 0, rv = 0;
    fd_set readfds;
    struct timeval tv;
    char cmd_buf[1025], reply_buf[1025];
    // Read Arguments
    int port = atoi(argv[1]);

    // Setup Sockets
    //printf("Setting up socket for cproxy\n");
    struct sockaddr_in address;
    int server_fd = setup_socket(&address, NULL, port);
    //printf("- socket for cproxy open\n");

    //printf("Binding to port for cproxy.\n");
    bind_and_listen(server_fd, &address, 5);
    //printf("- Bind to port for cproxy successed\n");

    //printf("Setting up socket for telnet (daemon)\n");
    struct sockaddr_in daemon_address;
    int server_teldaemon = setup_socket(&daemon_address, "127.0.0.1", 23);
    //printf("- socket for telnet (daemon) open\n");

    // Forcefully attaching socket to port
    connect_to_server(&daemon_address, server_teldaemon);

    // Client Loop        
    while (1) {
	// Accept client
	//printf("Accepting from cProxy\n");
	s1 = accept_client(server_fd, &address);
	s2 = server_teldaemon;
	//printf("- Accepted\n");
	int len1 = 0, len2 = 0;
	clock_t last_time=clock();
	while (1) {
	    // Receive messages from new_socket
	    FD_ZERO(&readfds);
	    // add our descriptors to the set
	    FD_SET(s1, &readfds);
	    FD_SET(s2, &readfds);
	    // find the largest descriptor, and plus one.
	    if (s1 > s2)
		n = s1 + 1;
	    else
		n = s2 + 1;

	    // wait until either socket has data ready to be recv()d (timeout 10.5 secs)                 
	    tv.tv_sec = 10;
	    tv.tv_usec = 500000;
	    rv = select(n, &readfds, NULL, NULL, &tv);
	    
	    if((double)(clock()-last_time)/CLOCKS_PER_SEC >= 1){
	        //send heartbeat;
		printf("HB\n");
		last_time = clock();
	    }

	    if (rv == -1) {
		perror("select");	// error occurred in select()
	    } else if (rv == 0) {
		;		//printf("Timeout occurred!  No data after 10.5 seconds.\n");
	    } else {
		// one or both of the descriptors have data
		//s1: cproxy
		if (FD_ISSET(s1, &readfds)) {
		    len1 = recv(s1, cmd_buf, sizeof(cmd_buf), 0);
		    //printf("Recved command from cproxy: %s\n", cmd_buf);
		    send(s2, cmd_buf, len1, 0);
		    memset(cmd_buf, 0, sizeof(cmd_buf));
		}
		//s2:telnet-daemon
		if (FD_ISSET(s2, &readfds)) {
		    len2 = recv(s2, reply_buf, sizeof(reply_buf), 0);
		    //printf("Recved reply from daemon: %s\n", reply_buf);
		    send(s1, reply_buf, len2, 0);
		    memset(reply_buf, 0, sizeof(reply_buf));
		}
	    }


	}			/*
				   if(len == 0)
				   printf("Nothing more from the client. Client has been shut down.\n");
				   if(len < 0)
				   printf("ERROR on RECV()!\n");
				 */
	close(s1);
	close(s2);
    }

    close(server_fd);
    close(server_teldaemon);
    return 0;
}
