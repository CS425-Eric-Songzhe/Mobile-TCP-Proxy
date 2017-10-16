//Description: Client side of the Moblie TCP Proxy project
//Authors: Eric Evans, Songzhe Zhu
//Date: Sep 29 2017
#include <sys/time.h>
#include <time.h>
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

    int len1 = 0;		//, len2 = 0;
    while (1) {
	// Accept telnet
	//printf("Accepting request from telnet\n");
	s1 = accept_client(sock_telnet, &addr_telnet);
	//printf("- telnet request accepted\n");

	// printf("Accepting request from daemon\n");
	s2 = sock;
	//printf("- daemon request accepted\n");

	// Set up heartbeat time interval checking
	struct timeval last, now;
	gettimeofday(&last, NULL);
	int hb_sent = 0;
	int hb_recv = 0;
	int last_hb = -1;
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


	    // wait until either socket has data ready to be recv()d (timeout 1.5 secs)
	    tv.tv_sec = 1;
	    tv.tv_usec = 500000;
	    rv = select(n, &readfds, NULL, NULL, &tv);

	    // Check if time to send heartbeat    
	    gettimeofday(&now, NULL);
	    double diff = (now.tv_sec - last.tv_sec) +
		((now.tv_usec - last.tv_usec) / 1000000.0);
	    //printf("diff: %f\n", diff);   
	    if (diff >= 1) {
		//send heartbeat;                                                                                                       
		char msg_HB[512] = { 0 };
		char *payload_HB = " ";
		int msg_len_HB = make_msg(msg_HB, HEARTBEAT, hb_sent, 1010,
					  sizeof(payload_HB), payload_HB);
		send(s2, msg_HB, msg_len_HB, 0);
		printf("print HB %d\n", hb_sent);
		gettimeofday(&last, NULL);
		hb_sent++;
	    }


	    if (rv == -1) {
		perror("select");	// error occurred in select()
	    } else if (rv == 0) {
		;		//printf("Timeout occurred!  No data after 10.5 seconds.\n");
	    } else {
		// one or both of the descriptors have data
		// s1: Telnet
		if (FD_ISSET(s1, &readfds)) {
		    len1 = recv(s1, cmd_buf, sizeof(cmd_buf), 0);
		    printf("Recved command from telnet: %s, \n %d\n",
			   cmd_buf, len1);
		    char msg_t[1025] = { 0 };
		    int msg_len_t =
			make_msg(msg_t, DATA, 0, 1010, len1, cmd_buf);
		    send(s2, msg_t, msg_len_t, 0);
		    memset(cmd_buf, 0, sizeof(cmd_buf));
		}
		// s2: Sproxy
		if (FD_ISSET(s2, &readfds)) {
		    recv(s2, reply_buf, sizeof(reply_buf), 0);

		    int type = -1;
		    int ackID = -1;
		    int sessionID = -1;
		    char payload_s[1025] = { 0 };
		    int paylen_s =
			parse_msg(reply_buf, &type, &ackID, &sessionID,
				  payload_s);

		    // If message is heartbeat, just record
		    if (type == HEARTBEAT) {
			printf("Received HB (%d) from %d\n", ackID,
			       sessionID);
			hb_recv++;
			if (ackID == last_hb) {
			    exit(0);
			}
			last_hb = ackID;
		    }
		    // else, if message is data, send payload
		    else if (type == DATA) {
			printf("Recieved Data from %d\n", sessionID);
			printf("Recved command from sproxy: %s\n, %d\n",
			       reply_buf, paylen_s);
			send(s1, payload_s, paylen_s, 0);
		    } else {
			printf("ERROR: unknown message type\n");
		    }

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
