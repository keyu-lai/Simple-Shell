#ifndef LINK_H_
#define LINK_H_

struct Node {
	char *str;
	struct Node *prev;
	struct Node *next;
};

struct Link {
	struct Node *head;
	struct Node *tail;
	int num;
};

void malloc_failure(void);
void Init_link(struct Link *link);
void insert(struct Link *link, const char *str);
void delete_str(struct Link *link, const char *str);
void delete_first(struct Link *link);
void clear(struct Link *link);
int size(struct Link *link);
void delete_last(struct Link *link);
int check_duplicate(struct Link *link, const char *str);

#endif
