//Description: Server side of the Moblie TCP Proxy project
//Authors: Eric Evans, Songzhe Zhu
//Date: Sep 29 2017
#include <sys/time.h>
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

    int s1_cproxy = 0, s2_daemon = 0, n = 0, rv = 0, sessionID = -1;
    fd_set readfds;
    struct timeval tv;
    char cmd_buf[9999], reply_buf[9999];
    // Read Arguments
    int port = atoi(argv[1]);

    // Setup Sockets
    //printf("Setting up socket for cproxy\n");
    //struct sockaddr_in address;
    //int server_fd = setup_socket(&address, NULL, port);
    //printf("- socket for cproxy open\n");

    //printf("Binding to port for cproxy.\n");
    //bind_and_listen(server_fd, &address, 5);
    //printf("- Bind to port for cproxy successed\n");

    //printf("Setting up socket for telnet (daemon)\n");
    struct sockaddr_in daemon_address;
    int server_teldaemon = setup_socket(&daemon_address, "127.0.0.1", 23);
    //printf("- socket for telnet (daemon) open\n");

    // Forcefully attaching socket to port
    connect_to_server(&daemon_address, server_teldaemon);

    // Client Loop        
    while (1) {
	// Setup Sockets
	printf("Setting up socket for cproxy\n");
	struct sockaddr_in address;
	int server_fd = setup_socket(&address, NULL, port);
	printf("- socket:%d for cproxy open\n", server_fd);

	// Forcefully attaching socket to the port 8080
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		       &opt, sizeof(opt))) {
	    perror("setsockopt");
	    exit(EXIT_FAILURE);
	}

	printf("Binding to port for cproxy.\n");
	bind_and_listen(server_fd, &address, 5);
	printf("- Bind to port for cproxy successed\n");

	// Accept client
	//printf("Accepting from cProxy\n");
	s1_cproxy = accept_client(server_fd, &address);
	s2_daemon = server_teldaemon;
	//printf("- Accepted\n");
	int len1 = 0, len2 = 0;

	// Set up heartbeat time interval checking
	struct timeval last, now, hb_time;
	gettimeofday(&last, NULL);
	int hb_sent = 0;
	int hb_recv = 0;
	int last_hb = -1;

	while (1) {
	    printf("---------------------------------------\n");
	    // Receive messages from new_socket
	    FD_ZERO(&readfds);
	    // add our descriptors to the set
	    FD_SET(s1_cproxy, &readfds);
	    FD_SET(s2_daemon, &readfds);
	    // find the largest descriptor, and plus one.
	    if (s1_cproxy > s2_daemon)
		n = s1_cproxy + 1;
	    else
		n = s2_daemon + 1;

	    // wait until either socket has data ready to be recv()d (timeout 1.5 secs)                 
	    tv.tv_sec = 1;	//10;
	    tv.tv_usec = 5;	//500000;
	    rv = select(n, &readfds, NULL, NULL, &tv);

	    // Check if time to send heartbeat
	    gettimeofday(&now, NULL);
	    double diff = (now.tv_sec - last.tv_sec) +
		((now.tv_usec - last.tv_usec) / 1000000.0);
	    //printf("diff: %f\n", diff);
	    if (diff >= 1) {
		//send heartbeat;
		char msg_HB[9999] = { 0 };
		char *payload_HB = " ";
		int msg_len_HB =
		    make_msg(msg_HB, HEARTBEAT, hb_sent, sessionID,
			     sizeof(payload_HB), payload_HB);
		usleep(500);
		send(s1_cproxy, msg_HB, msg_len_HB, 0);
		printf("send HB %d\n", hb_sent);
		gettimeofday(&last, NULL);
		hb_sent++;
	    }
	    // Detect if timeout by heartbeat
	    if (last_hb >= 0) {
		gettimeofday(&now, NULL);
		double diff_hb =
		    (now.tv_sec - hb_time.tv_sec) +
		    ((now.tv_usec - hb_time.tv_usec) / 1000000.0);
		if (diff_hb >= 3 /*&& sessionID > 0 */ ) {
		    printf("HB Timeout occured\n");
		    printf("Socket to cproxy closed.\n");
		    close(s1_cproxy);
		    //struct sockaddr_in new_address;
		    //server_fd = setup_socket(&new_address, NULL, port);
		    //printf("- socket:%d for cproxy open\n", server_fd);
/*
                    // Forcefully attaching socket to the port 8080
                    int opt = 1;
                    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
                    perror("setsockopt");
                    exit(EXIT_FAILURE);
                    }
*/
		    printf("Listenning:  %d\n", server_fd);
		    listen(server_fd, 5);
		    printf("trying to accept\n");
		    s1_cproxy = accept_client(server_fd, &address);
		    printf("accepted!!!!!!!\n");
		    // continue;
		    //break;
		}
	    }


	    if (rv == -1) {
		perror("select");	// error occurred in select()
	    } else if (rv == 0) {
		;		//printf("Timeout occurred!  No data after 10.5 seconds.\n");
	    } else {
		// one or both of the descriptors have data
		//s1_cproxy: cproxy
		if (FD_ISSET(s1_cproxy, &readfds)) {
		    //printf("wait for cproxy\n");
		    len1 = recv(s1_cproxy, cmd_buf, sizeof(cmd_buf), 0);
		    if (len1 > 0) {
			printf("Recvd from s1_cproxy: %d\n", len1);
			int i = 0;
			for (i = 0; i < len1; i++) {
			    printf("%c", cmd_buf[i]);
			}
			printf("\n");

			char *pkt_buf = cmd_buf;	// for shifting
			int pkt_len = len1;	// keeping track of where to start parsing
			int go_again = 0;	//1 if multiple messages
			// parse each message from packet
			do {
			    int type = -1;
			    int ackID = -1;
			    int sessionID_C = -1;
			    char payload_c[9999] = { 0 };
			    int paylen_c =
				parse_msg(pkt_buf, &type, &ackID,
					  &sessionID_C,
					  payload_c);
			    if (sessionID != sessionID_C) {
				printf("New session ID: change %d to %d\n",
				       sessionID, sessionID_C);
				sessionID = sessionID_C;
			    } else {
				printf("Reconnected with: %d\n",
				       sessionID_C);
			    }
			    // If message is heartbeat, just record
			    if (type == HEARTBEAT) {
				//if (ackID != 0)
				printf("Received HB (%d) from %d\n", ackID,
				       sessionID_C);
				//hb_recv++;
				//if (ackID != last_hb) {
				// record time of this heartbeat
				gettimeofday(&hb_time, NULL);
				//last_hb = ackID;
				//}
				hb_recv++;
				last_hb++;
			    } else if (type == DATA) {
				printf("Recieved Data from %d\n",
				       sessionID_C);
				//printf("Recved command from cproxy: %s\n, %d\n", cmd_buf, paylen_c);
				usleep(500);
				send(s2_daemon, payload_c, paylen_c, 0);
			    } else if (type == ACK) {
				printf
				    ("Recvd Acknowledgement from %d, %d\n",
				     sessionID_C, ackID);
			    } else {
				//printf("ERROR: unknown message type\n");
				;
			    }

			    // Check if there are remaining multiple messages
			    int hdr_and_pay = HDR_LEN + paylen_c;
			    if (hdr_and_pay < pkt_len) {
				go_again = 1;
				// truncate pkt_len
				pkt_len = pkt_len - hdr_and_pay;
				// shift beginning of cmd_buf to next message
				pkt_buf += hdr_and_pay;
				printf
				    ("~~WOW multiple messages! going to go again.~~\n");
			    } else {
				go_again = 0;
			    }

			} while (go_again);

			//printf("Recved command from cproxy: %s\n", cmd_buf);
			//send(s2_daemon, cmd_buf, len1, 0);
			memset(cmd_buf, 0, sizeof(cmd_buf));
		    } else {
			printf("proxy connection failed.\n");
			/*printf("Trying to reconnect to cproxy -----------------\n");
			   // Setup Sockets
			   printf("Setting up socket for cproxy\n");
			   struct sockaddr_in new_address;
			   server_fd = setup_socket(&new_address, NULL, port);
			   printf("- socket:%d for cproxy open\n", server_fd);

			   // Forcefully attaching socket to the port 8080
			   int opt = 1;
			   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			   &opt, sizeof(opt))) {
			   perror("setsockopt");
			   exit(EXIT_FAILURE);
			   }

			   printf("listening to port for cproxy to reconnect.\n");
			   listen(server_fd, 5);
			   printf("-Listen to port for cproxy reconnect successed\n");

			   // Accept client
			   printf("Accepting from cProxy\n");
			   s1_cproxy = accept_client(server_fd, &new_address);
			   printf("Accepted to new cproxy\n"); */
		    }
		}
		//s2_daemon:telnet-daemon
		if (FD_ISSET(s2_daemon, &readfds)) {
		    //printf("wait for daemon\n");
		    len2 =
			recv(s2_daemon, reply_buf, sizeof(reply_buf), 0);
		    if (len2 > 0) {
			printf("Recvd from s2_daemon: %d\n", len2);
			int i = 0;
			for (i = 0; i < len2; i++) {
			    printf("%c", reply_buf[i]);
			}
			printf("\n");

			// Send Acknowledgment
			char msg_ack[9999] = { 0 };
			char *ack_str = "ack";
			int msg_len_ack =
			    make_msg(msg_ack, ACK, 0, sessionID,
				     strlen(ack_str), ack_str);
			usleep(500);
			send(s1_cproxy, msg_ack, msg_len_ack, 0);
			printf("sent acknow\n");

			// make message
			char msg_d[9999] = { 0 };
			int msg_len_d =
			    make_msg(msg_d, DATA, 0, sessionID, len2,
				     reply_buf);

			usleep(500);
			send(s1_cproxy, msg_d, msg_len_d, 0);
			memset(reply_buf, 0, sizeof(reply_buf));
		    } else {
			printf
			    ("Telnet-daemon connection failed, reconnecting ...\n");
			/*close(s2_daemon);
			   s2_daemon = setup_socket(&daemon_address, "127.0.0.1", 23);
			   connect_to_server(&daemon_address, server_teldaemon); */
		    }
		}
	    }


	}			/*
				   if(len == 0)
				   printf("Nothing more from the client. Client has been shut down.\n");
				   if(len < 0)
				   printf("ERROR on RECV()!\n");
				 */
	close(s1_cproxy);
	//close(s2_daemon);
	//close(server_fd);
    }
    close(s2_daemon);
    //close(server_fd);
    close(server_teldaemon);
    return 0;
}
