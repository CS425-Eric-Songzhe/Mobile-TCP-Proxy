#include <stdlib.h>
#include <string.h>
#include "queue.h"

Msg* new_msg(char* content, int length) {
	Msg *head = (Msg*)malloc(sizeof(Msg));
	head->content = (char*)malloc(sizeof(char) * length);
	strncpy(head->content, content, length);
	head->next = NULL;
	return head;
}

Msg* create_queue() {
	Msg *head = (Msg*)malloc(sizeof(Msg));
	head->next = NULL;
	return head;
}

void enqueue(Msg *head, Msg *node) {
	Msg *cur = head;
	while (cur->next != NULL)
		cur = cur->next;
	cur->next = node;
}

Msg* dequeue(Msg *head) {
	Msg *cur = head->next;
	if (cur != NULL)
		head->next = head->next->next;
	return cur;
}

int claer(Msg *head) {
	return head->next == NULL ? 1 : 0;
}
