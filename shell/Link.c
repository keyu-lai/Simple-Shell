#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "Link.h"

/* print out error message and exit when malloc() fails. */
void malloc_failure(void)
{
	printf("error: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
}

/* Initiate a doubly linked list. */
void Init_link(struct Link *link)
{
	link->head = malloc(sizeof(struct Node));
	if (link->head == NULL)
		malloc_failure();
	link->tail = malloc(sizeof(struct Node));
	if (link->tail == NULL)
		malloc_failure();
	link->head->str = malloc(sizeof(char));
	if (link->head->str == NULL)
		malloc_failure();
	link->head->prev = NULL;
	link->head->next = link->tail;
	link->tail->str = malloc(sizeof(char));
	if (link->tail->str == NULL)
		malloc_failure();
	link->tail->prev = link->head;
	link->tail->next = NULL;
	link->num = 0;
}

/* If a string exists in the linked list, then return 1. */
int check_duplicate(struct Link *link, const char *str)
{
	struct Node *p = link->head;

	while ((p = p->next) != link->tail) {
		if (!strcmp(p->str, str))
			return 1;
	}
	return 0;
}

/* Insert a string after the last element. */
void insert(struct Link *link, const char *str)
{
	struct Node *p = link->tail->prev;

	link->tail->prev = malloc(sizeof(struct Node));
	if (link->tail->prev == NULL)
		malloc_failure();
	link->tail->prev->str = malloc((strlen(str) + 1) * sizeof(char));
	if (link->tail->prev->str == NULL)
		malloc_failure();
	strcpy(link->tail->prev->str, str);
	p->next = link->tail->prev;
	p->next->next = link->tail;
	p->next->prev = p;
	++(link->num);
}

/*
 * If a string is found in the linked list, then delete it.
 * Otherwise, do nothing.
 */
void delete_str(struct Link *link, const char *str)
{
	struct Node *p = link->head;
	struct Node *tmp;

	while ((p = p->next) != link->tail) {
		if (!strcmp(p->str, str)) {
			p->next->prev = p->prev;
			p->prev->next = p->next;
			tmp = p;
			p = p->prev;
			free(tmp->str);
			free(tmp);
			--(link->num);
		}
	}
}

/* Delete the last node of a linked list if it exists. */
void delete_last(struct Link *link)
{
	struct Node *p = link->tail->prev;

	if (p == link->head)
		return;
	link->tail->prev = p->prev;
	p->prev->next = p->next;
	free(p->str);
	free(p);
	--(link->num);
}

/* Delete the first node of a linked list if it exists. */
void delete_first(struct Link *link)
{
	struct Node *tmp = link->head->next;

	if (tmp == link->tail)
		return;
	link->head->next = tmp->next;
	tmp->next->prev = link->head;
	free(tmp->str);
	free(tmp);
	--(link->num);
}

/* Reclaim all the space dynamically allocated. */
void clear(struct Link *link)
{
	struct Node *p = link->head;
	struct Node *tmp;

	while (p != NULL) {
		tmp = p->next;
		free(p->str);
		free(p);
		p = tmp;
	}
	link->num = 0;
}

/* Return the size of a linked list. */
int size(struct Link *link)
{
	return link->num;
}

