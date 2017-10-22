#include <stdlib.h>
#include <string.h>
#include "queue.h"

msg* new_msg(char* content, int length) {
	msg *head = (msg*)malloc(sizeof(msg));
	head->content = (char*)malloc(sizeof(char) * length);
	strncpy(head->content, content, length);
	head->next = NULL;
	return head;
}

msg* create_queue() {
	msg *head = (msg*)malloc(sizeof(msg));
	head->next = NULL;
	return head;
}

void enqueue(msg *head, msg *node) {
	msg *cur = head;
	while (cur->next != NULL)
		cur = cur->next;
	cur->next = node;
}

msg* pop(msg *head) {
	msg *cur = head->next;
	if (cur != NULL)
		head->next = head->next->next;
	return cur;
}

int claer(msg *head) {
	return head->next == NULL ? 1 : 0;
}
