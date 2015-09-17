#include <string.h>
#include <stdlib.h>
#include "Link.h"

void Init_link(Link *link) 
{
	link->head = malloc(sizeof(Node));
	link->tail = malloc(sizeof(Node));
	link->head->str = malloc(sizeof(char));
	link->head->prev = NULL;
	link->head->next = link->tail;
	link->tail->str = malloc(sizeof(char));;
	link->tail->prev = link->head;
	link->tail->next = NULL;
	link->num = 0;
}

void insert(Link *link, const char *str)
{
	Node *p = link->tail->prev;
	link->tail->prev = malloc(sizeof(Node));
	link->tail->prev->str = malloc((strlen(str) + 1) * sizeof(char));
	strcpy(link->tail->prev->str, str);
	p->next = link->tail->prev;
	p->next->next = link->tail;
	p->next->prev = p;
	link->num++;
}

void delete_str(Link *link, const char *str) 
{
	Node *p = link->head;
	Node *tmp;
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

void delete_last(Link *link)
{
	Node *p = link->tail->prev;
	if (p == link->head)
		return;
	link->tail->prev = p->prev;
	p->prev->next = p->next;
	free(p->str);
	free(p);
	--(link->num);	
}

void delete_first(Link *link)
{
	Node *tmp = link->head->next;
	if (tmp == link->tail)
		return;
	link->head->next = tmp->next;
	tmp->next->prev = link->head;
	free(tmp->str);
	free(tmp);
	--(link->num);	
}

void clear(Link *link)
{
	Node *p = link->head;
	Node *tmp;
	while (p != NULL) {
		tmp = p->next;
		free(p->str);
		free(p);
		p = tmp;
	}
	link->num = 0;
}

int size(Link *link)
{
	return link->num;
}

