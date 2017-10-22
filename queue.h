#ifndef QUEUE_H
#define QUEUE_H

typedef struct msg {
	char* content;
	struct msg* next;
}msg;

msg* create_queue();
msg* new_msg(char* content, int length);
void enqueue(msg *head, msg *node);
msg* dequeue(msg *head);
int clear(msg *head);

#endif
