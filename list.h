#include <stdio.h>
#include <stdlib.h>


typedef struct Node {
	void *data;
	struct Node *next;
	struct Node *prev;
} Node;

typedef struct List {
	int count;
	Node *first;
	Node *last;
} List;


List *list_create(void);
Node *list_add_node(List* l, void *data);
Node *list_get_next_node(List *l, Node *n);
int   list_delete_node(List *l, Node *n);
int   list_delete_last_node(List *l);

