
/*
 * Has functions for reading and writing messages with headers.
 */

#include <stdio.h>
#include <string.h>
#include "mymessages.h"

#define TEST_MSG 0


/*                                                  
 * return the 4 bytes of buffer as an int at index
 */
int decode_int(char *buffer, int index)
{

    int num = 0;
    num = buffer[index] << 24 |
	(buffer[index + 1] & 0xFF) << 16 |
	(buffer[index + 2] & 0xFF) << 8 | (buffer[index + 3] & 0xFF);

    return num;
}


/*
 * encode num into four bytes in msg at index
 */
void encode_int(int n, char *msg, int index)
{

    long num = (long) n;

    msg[index] = (num >> 24) & 0xFF;
    msg[index + 1] = (num >> 16) & 0xFF;
    msg[index + 2] = (num >> 8) & 0xFF;
    msg[index + 3] = num & 0xFF;

}


/*
 * prepend length of payload to payload. this is the message to be sent
 */
int make_msg(char *msg, int type, int ackID, int sessionID, int paylen,
	     char *payload)
{

    // Type - heartbeat or data
    encode_int(type, msg, 0);

    // AckID
    encode_int(ackID, msg, 4);

    // SessionID
    encode_int(sessionID, msg, 8);

    // Paylen
    encode_int(paylen, msg, 12);

    // Append Payload
    memcpy(&msg[HDR_LEN], payload, paylen);

    printf("Make data: %s\n", &msg[HDR_LEN]);

    return HDR_LEN + paylen;
}


/*
 * parse message and pull out fields type, ackID, sessionID, and payload
 */
int parse_msg(char *msg, int *type, int *ackID, int *sessionID,
	      char *payload)
{

    // Type - heartbeat or data
    *type = decode_int(msg, 0);

    // AckID
    *ackID = decode_int(msg, 4);

    // SessionID
    *sessionID = decode_int(msg, 8);

    // Paylen
    int paylen = decode_int(msg, 12);

    printf("Recv data: %s\n", &msg[HDR_LEN]);

    // Payload
    memcpy(payload, &msg[HDR_LEN], paylen);

    printf("After parse, payload is: %s\n", payload);

    return paylen;
}


#if TEST_MSG == 1
int main(int argc, char const *argv[])
{

    // Make Message
    int type = 1;
    int ackNum = 558;
    int sessionID = 78;
    char *payload = "Hello World!";
    char msg[1025] = { 0 };

    int l =
	make_msg(msg, type, ackNum, sessionID, strlen(payload), payload);

    int i = 0;
    for (i = 0; i < l; i++)
	printf("%d", msg[i]);
    printf("\n");

    // Parse Message
    char b_payload[1025] = { 0 };
    int b_type = 0;
    int b_ackNum = 0;
    int b_sID = 0;

    int len = parse_msg(msg, &b_type, &b_ackNum, &b_sID, b_payload);

    // Compare
    printf("%d|%d\n", type, b_type);
    printf("%d|%d\n", ackNum, b_ackNum);
    printf("%d|%d\n", sessionID, b_sID);
    printf("%d|%d\n", (int) strlen(payload), len);
    printf("%s|%s\n", payload, b_payload);

    return 0;
}
#endif
