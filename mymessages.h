#ifndef MYMESSAGES_H_INCLUDED
#define MYMESSAGES_H_INCLUDED

/*
 * prepend length of payload to payload. this is the message to be sent
 */
void make_msg(char *msg, long len, char *input);

/*
 * replace the last character with '\0'
 */
void remove_last_char(char *buffer);

/*                                                                                                                                      
 * return the first 4 bytes of buffer as an int                                                                                         
 */
int get_length(char *buffer);

#endif
