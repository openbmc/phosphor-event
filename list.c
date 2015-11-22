#include <stdio.h>
#include <stdlib.h>
#include "list.h"

List *list_create(void) {
	return calloc(1, sizeof(List));
}

Node *list_add_node(List* l, void *data) {

	Node *n = calloc(1, sizeof(Node));

	if(l->last == NULL) {
		l->first = n;
		l->last  = n;
	} else {
		l->last->next = n;
		n->prev = l->last;
		l->last = n;
	}

	l->count++;
	n->data = data;

	return n;
}

int list_delete_node(List *l, Node *n) {
	void *result = NULL;
	Node *after, *before;

	if(n == l->first && n == l->last) {
		l->first = NULL;
		l->last  = NULL;

	} else if(n == l->first) {
		l->first       = n->next;
		l->first->prev = NULL;

	} else if (n == l->last) {
		l->last       = n->prev;
		l->last->next = NULL;

	} else {
		after = n->next;
		before = n->prev;
		after->prev = before;
		before->next = after;
	}

	l->count--;
	
	free(n);

	return l->count;
}


int list_delete_last_node(List *l)  {

	list_delete_node(l, l->last);
}

Node * list_get_next_node(List *l, Node *n) {

	Node *t;

	if (n==NULL) {
		t = l->first;
	} else {
		t = n->next;
	}

	return t;
}