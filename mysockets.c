/*
 * Comprises of socket functions with perrors, exits, 
 * and specialized parameters.
 */

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
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("Socket creation error");
	exit(EXIT_FAILURE);
    }
    // Client Option
    if (ip != NULL) {
	memset(serv_addr, '0', sizeof(*serv_addr));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_port = htons(port);
	serv_addr->sin_addr.s_addr = inet_addr(ip);
    }
    // Server Option
    else {
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons(port);
    }

    return sock;
}


/*
 * bind port to server_fd socket and setup listening queue
 */
void
bind_and_listen(int server_fd, struct sockaddr_in *address, int backlog)
{
    if (bind(server_fd, (struct sockaddr *) address, sizeof(*address)) < 0) {
	printf("BIND ERROR: %s\n", strerror(errno));
	perror("bind failed");
	exit(EXIT_FAILURE);
    }
    if (listen(server_fd, backlog) < 0) {
	printf("Listen ERROR: %s\n", strerror(errno));
	perror("listen");
	exit(EXIT_FAILURE);
    }

}

/*
 * connect to server with serv_addr
 */
int connect_to_server(struct sockaddr_in *serv_addr, int sock)
{

    if (connect(sock, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) <
	0) {
	perror("Connection Failed");
	printf("%d\n", sock);
	return 0;
	//exit(EXIT_FAILURE);
    }
    //printf("telnet connected at: %d\n", sock);

    return 1;
}



/*
 * accept a new client and return socket value
 */
int accept_client(int server_fd, struct sockaddr_in *address)
{
    int sock = 0;
    int addrlen = sizeof(*address);
    if ((sock =
	 accept(server_fd, (struct sockaddr *) address,
		(socklen_t *) & addrlen)) < 0) {
	perror("accept failed");
	exit(EXIT_FAILURE);
    }

    return sock;
}
