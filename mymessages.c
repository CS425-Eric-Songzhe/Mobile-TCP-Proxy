/*
 * Has functions for reading and writing messages with headers.
 */

#include <string.h>

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


/*
 * replace the last character with '\0'
 */
void remove_last_char(char *buffer)
{

    int len = strlen(buffer);
    buffer[len - 1] = '\0';
}


/*                                                                                                                                      
 * return the first 4 bytes of buffer as an int                                                                                         
 */
int get_length(char *buffer)
{

    int num = 0;

    num = buffer[0] << 24 |
	(buffer[1] & 0xff) << 16 |
	(buffer[2] & 0xff) << 8 | (buffer[3] & 0xff);

    return num;
}
