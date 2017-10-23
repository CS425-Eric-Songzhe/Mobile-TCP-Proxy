#ifndef MYMESSAGES_H_INCLUDED
#define MYMESSAGES_H_INCLUDED

#define HEARTBEAT 2
#define DATA 1
#define HDR_LEN 20

/*
 * prepend length of payload to payload. this is the message to be sent
 */
int make_msg(char *msg, int type, int ackID, int sessionID, int paylen, int seqID,
	     char *payload);


/*
 * parse message and pull out fields type, ackID, sessionID, and payload
 */
int parse_msg(char *msg, int *type, int *ackID, int *sessionID, int *seqID,
	      char *payload);


#endif
