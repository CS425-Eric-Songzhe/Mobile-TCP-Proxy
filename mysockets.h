#ifndef MYSOCKETS_H_INCLUDED
#define MYSOCKETS_H_INCLUDED

/*
 * set up socket and set serv_addr fields 
 */
int setup_socket(struct sockaddr_in *serv_addr, char const *ip, int port);

/*
 * bind port to server_fd socket and setup listening queue
 */
void bind_and_listen(int server_fd, struct sockaddr_in *address,
		     int backlog);

/*
 * connect to server with serv_addr
 */
void connect_to_server(struct sockaddr_in *serv_addr, int sock);

/*
 * accept a new client and return socket value
 */
int accept_client(int server_fd, struct sockaddr_in *address);

#endif
