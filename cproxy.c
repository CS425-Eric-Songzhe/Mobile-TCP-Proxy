//Description: Client side of the Moblie TCP Proxy project
//Authors: Eric Evans, Songzhe Zhu
//Date: Sep 29 2017
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <sys/select.h>
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
    char const *ip = argv[2];	// server ip address
    int port_sproxy = atoi(argv[3]);	// server port
    int port_telnet = atoi(argv[1]);	// telnet port

    // Setup Sockets
    //printf("Setting up socket for sproxy\n");
    struct sockaddr_in serv_addr;
    int sock = setup_socket(&serv_addr, ip, port_sproxy);
    //  printf("- socket for sproxy open\n");

    //printf("Listening for client connect request\n");
    connect_to_server(&serv_addr, sock);
    //printf("- connected to client\n");

    //printf("Setting up socket for telnet\n");
    struct sockaddr_in addr_telnet;
    int sock_telnet = setup_socket(&addr_telnet, "127.0.0.1", port_telnet);
    //printf("- socket for telnet open\n");

    // Connect with Telnet
    //printf("Listening for telnet connect request\n");
    bind_and_listen(sock_telnet, &addr_telnet, 5);
    //connect_to_server(&addr_telnet, sock_telnet);
    //printf("- connected to telnet\n");

    int len1 = 0, len2 = 0;
    while (1) {
	// Accept telnet
	//printf("Accepting request from telnet\n");
	s1 = accept_client(sock_telnet, &addr_telnet);
	//printf("- telnet request accepted\n");

	// printf("Accepting request from daemon\n");
	s2 = sock;
	//printf("- daemon request accepted\n");

	while (1) {
	    // clear the set ahead of time
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

	    if (rv == -1) {
		perror("select");	// error occurred in select()
	    } else if (rv == 0) {
		;		//printf("Timeout occurred!  No data after 10.5 seconds.\n");
	    } else {
		// one or both of the descriptors have data
		if (FD_ISSET(s1, &readfds)) {
		    len1 = recv(s1, cmd_buf, sizeof(cmd_buf), 0);
		    //printf("Recved command from telnet: %s\n", cmd_buf);
		    send(s2, cmd_buf, len1, 0);
		    memset(cmd_buf, 0, sizeof(cmd_buf));
		}
		if (FD_ISSET(s2, &readfds)) {
		    len2 = recv(s2, reply_buf, sizeof(reply_buf), 0);
		    //printf("Recved reply from server: %s\n", reply_buf);
		    send(s1, reply_buf, len2, 0);
		    memset(reply_buf, 0, sizeof(reply_buf));
		}
	    }

	}
	close(s1);
	close(s2);
    }

    close(sock_telnet);
    return 0;
}
