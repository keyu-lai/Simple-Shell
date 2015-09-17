#ifndef LINK_H_
#define LINK_H_

typedef struct Node Node;
typedef struct Link Link;

struct Node 
{
	char *str;
	Node *prev;
	Node *next;	
};

struct Link
{
	Node *head;
	Node *tail;
	int num;
};

void Init_link(Link *link);
void insert(Link *link, const char *str);
void delete_str(Link *link, const char *str);
void delete_first(Link *link);
void clear(Link *link);
int size(Link *link);
void delete_last(Link *link);

#endif
