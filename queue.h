#ifndef QUEUE_H
#define QUEUE_H

typedef struct Msg {
	char* content;
	struct Msg* next;
}Msg;

Msg* create_queue();
Msg* new_msg(char* content, int length);
void enqueue(Msg *head, Msg *node);
Msg* dequeue(Msg *head);
int clear(Msg *head);

#endif
