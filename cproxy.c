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
    int s1_telnet = 0, s2_sproxy = 0, n = 0, rv = 0;
    fd_set readfds;
    struct timeval tv;
    char cmd_buf[9999], reply_buf[9999];

    // Read Arguments
    char const *ip = argv[2];	// server ip address
    int port_sproxy = atoi(argv[3]);	// server port
    int port_telnet = atoi(argv[1]);	// telnet port

    // Setup Sockets
    //printf("Setting up socket for sproxy\n");
    //struct sockaddr_in serv_addr;
    //int sock = setup_socket(&serv_addr, ip, port_sproxy);
    //  printf("- socket for sproxy open\n");

    //printf("Listening for client connect request\n");
    //connect_to_server(&serv_addr, sock);
    //int sessionID = rand();
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

    // Accept telnet
    //printf("Accepting request from telnet\n");
    s1_telnet = accept_client(sock_telnet, &addr_telnet);
    //printf("- telnet request accepted\n");
    //printf("- telnet request accepted\n");
    int sessionID = rand();


    int len1 = 0, len2 = 0;
    while (1) {

	// Setup Sockets
	printf("Setting up socket for sproxy\n");
	struct sockaddr_in serv_addr;
	int sock = setup_socket(&serv_addr, ip, port_sproxy);
	printf("- socket:%d  for sproxy open\n", sock);

	// Forcefully attaching socket to the port 8080
    	int opt=1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    	{
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}


	printf("Listening for client connect request\n");
	connect_to_server(&serv_addr, sock);
	printf("- connected to client\n");

	// sproxy
	s2_sproxy = sock;

	// Set up heartbeat time interval checking
	struct timeval last, now, hb_time;
	gettimeofday(&last, NULL);
	int hb_sent = 0;
	int hb_recv = 0;
	int last_hb = -1;
	while (1) {
	    printf("---------------------------------------\n");
	    // clear the set ahead of time
	    FD_ZERO(&readfds);

	    // add our descriptors to the set
	    FD_SET(s1_telnet, &readfds);
	    FD_SET(s2_sproxy, &readfds);

	    // find the largest descriptor, and plus one.
	    if (s1_telnet > s2_sproxy)
		n = s1_telnet + 1;
	    else
		n = s2_sproxy + 1;


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
		char msg_HB[9999] = { 0 };
		char *payload_HB = " ";
		int msg_len_HB =
		    make_msg(msg_HB, HEARTBEAT, hb_sent, sessionID,
			     sizeof(payload_HB), payload_HB);
		usleep(500);
		send(s2_sproxy, msg_HB, msg_len_HB, 0);
		printf("print HB %d\n", hb_sent);
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
		    close(s2_sproxy);
		    printf("Socket to sproxy closed. Reconnecting a new one -------------------\n");
		    //break;
		    struct sockaddr_in new_addr;
		    s2_sproxy = setup_socket(&new_addr, ip, port_sproxy);
		    
		    int try = 1;
		    do{
		      printf("Attempting to reconnect...\n");
		      try = connect_to_server(&new_addr, s2_sproxy);
		      //usleep(5000);
		    } while(try == 0);
		    gettimeofday(&hb_time, NULL);
		    //diff_hb = 0; //avoid reconnection unti recv hb
		}
	    }


	    if (rv == -1) {
		perror("select");	// error occurred in select()
	    } else if (rv == 0) {
		;		//printf("Timeout occurred!  No data after 10.5 seconds.\n");
	    } else {
		// one or both of the descriptors have data
		// s1_telnet: Telnet
		if (FD_ISSET(s1_telnet, &readfds)) {
		    len1 = recv(s1_telnet, cmd_buf, sizeof(cmd_buf), 0);
		    if(len1 > 0){
		    printf("Recvd from s1_telnet: %d\n", len1);
		    int i = 0;
		    for (i = 0; i < len1; i++) {
			printf("%c", cmd_buf[i]);
		    }
		    printf("\n");


		    // Send Acknowledgment                                     
		    char msg_ack[9999] = { 0 };
		    char *ack_str = "ack";
		    int msg_len_ack = make_msg(msg_ack, ACK, 0, sessionID,
					       strlen(ack_str), ack_str);
		    usleep(500);
		    send(s2_sproxy, msg_ack, msg_len_ack, 0);
		    printf("sent acknow\n");

		    // make message
		    char msg_t[9999] = { 0 };
		    int msg_len_t =
			make_msg(msg_t, DATA, 0, sessionID, len1, cmd_buf);
		    usleep(500);
		    send(s2_sproxy, msg_t, msg_len_t, 0);
		    memset(cmd_buf, 0, sizeof(cmd_buf));
		    }
		    else{
			printf("Telnet connect failed, try to reconnect.\n");
			/*close(s1_telnet);
			struct sockaddr_in new_addr_telnet;
    			int new_sock_telnet = setup_socket(&new_addr_telnet, "127.0.0.1", port_telnet);
    			//printf("- socket for telnet open\n");

    			// Connect with Telnet
    			printf("Listening for telnet connect request\n");
    			listen(new_sock_telnet, 5);
    			//connect_to_server(&addr_telnet, sock_telnet);
    			printf("- connected to telnet\n");
			
    			// Accept telnet
    			printf("Accepting request from telnet\n");
    			s1_telnet = accept_client(new_sock_telnet, &new_addr_telnet);
			printf("Accepted with new telnet at sock: %d\n", s1_telnet);
			close(s2_sproxy);
                   	s2_sproxy = setup_socket(&serv_addr, ip, port_sproxy);
                   	printf("- socket:%d  for sproxy open\n", s2_sproxy);

                   	printf("Listening for client connect request\n");
                   	connect_to_server(&serv_addr, s2_sproxy);
                   	printf("- connected to client\n");
			
		        sessionID = rand();
			continue;*/
		    }
		}
		// s2_sproxy: Sproxy
		if (FD_ISSET(s2_sproxy, &readfds)) {
		    len2 =
			recv(s2_sproxy, reply_buf, sizeof(reply_buf), 0);
		    if(len2 > 0){
		    printf("Recvd from s2_cproxy: %d\n", len2);
		    int i = 0;
		    for (i = 0; i < len2; i++) {
			printf("%c", reply_buf[i]);
		    }
		    printf("\n");

		    char *pkt_buf = reply_buf;	// for shifting   
		    int pkt_len = len2;	// keeping track of where to start parsing
		    int go_again = 0;	//1 if multiple messages    
		    // parse each message from packet
		    do {
			int type = -1;
			int ackID = -1;
			int sessionID_S = -1;
			char payload_s[9999] = { 0 };
			int paylen_s = parse_msg(pkt_buf, &type, &ackID,
						 &sessionID_S,
						 payload_s);

			// If message is heartbeat, just record
			if (type == HEARTBEAT) {
			    printf("Received HB (%d) from %d\n", ackID,
				   sessionID_S);
			    gettimeofday(&hb_time, NULL);
			    hb_recv++;
			    last_hb++;
			    //if(ackID == last_hb){
			    //      exit(0);
			    //}
			    //last_hb = ackID;
			}
			// else, if message is data, send payload
			else if (type == DATA) {
			    printf("Recieved Data from %d\n", sessionID_S);
			    printf
				("Recved command from sproxy: |%.*s|\n, %d\n",
				 (int) sizeof(reply_buf), reply_buf,
				 paylen_s);
			    usleep(500);
			    send(s1_telnet, payload_s, paylen_s, 0);
			} else if (type == ACK) {
			    printf("Recvd Acknowledgement from %d, %d\n",
				   sessionID_S, ackID);
			} else {
			    printf("ERROR: unknown message type: %d\n",
				   type);
			}

			// Check if there are remaining multiple messages      
			int hdr_and_pay = HDR_LEN + paylen_s;
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

		    memset(reply_buf, 0, sizeof(reply_buf));
		}
		else{
		   printf("proxy connection fails, trying to reconnect ---------------------\n");
		   close(s2_sproxy);
		   s2_sproxy = setup_socket(&serv_addr, ip, port_sproxy);
        	   printf("- socket:%d  for sproxy open\n", sock);

        	   printf("Listening for client connect request\n");
        	   connect_to_server(&serv_addr, sock);
        	   printf("- connected to client\n");
		}
		}
	    }

	}
	//close(s2_sproxy);
	close(sock);
	close(s2_sproxy);
    }
    //close(s2_sproxy);
    close(s1_telnet);
    close(sock_telnet);
    return 0;
}
