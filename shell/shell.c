#include <stdio.h>     
#include <stdlib.h>   
#include <unistd.h> 
#include <string.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "Link.h"


char *read_line();
char **parse_command(char *line, int *num, char *delim);
void shell_pwd(int num);
void shell_cd(char **args, int num);
void print_paths() ;
void shell_path(char **args, int num);
void search(char** args, int num);
void shell_execute(char** args, int num);
void shell_record(char *line);
void print_history();
void shell_history(char **args, int num);

Link paths;
Link history;

int main(int argc, char **argv)
{ 
	int num;
	char *line;
	char **args;
	
	Init_link(&paths);
	Init_link(&history);
		
	while(1) {
		printf("$");
		line = read_line();
		shell_record(line);
		args = parse_command(line, &num, " ");
		if (num == 0) {
			free(line);
			free(args);
			continue;
		}
		if (!strcmp(args[0], "exit")) {
			free(line);
			free(args);
			break;
		} else if (!strcmp(args[0], "cd"))
			shell_cd(args, num);
		else if (!strcmp(args[0], "pwd"))
			shell_pwd(num);
		else if (!strcmp(args[0], "path"))
			shell_path(args, num);
		else if (!strcmp(args[0], "history"))
			shell_history(args, num);
		else
			shell_execute(args, num);
		free(line);
		free(args);		
	}
	clear(&paths);
	clear(&history);
   	return EXIT_SUCCESS;
}

void shell_execute(char** args, int num) 
{
	int pid;
	pid = fork();
	if (pid < 0) {
		printf("error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);			
	} 
	if (pid == 0) {
		search(args, num);
	}
	else {
		int return_code;
		while (pid != wait(&return_code));	
	}	
}

void search(char** args, int num)
{
	char exc_dir[PATH_MAX];
	Node *p = paths.head;
	execl(args[0], args[0], (char*)NULL);
	while ((p = p->next) != paths.tail) {
		strcpy(exc_dir, p->str);
		if (exc_dir[strlen(exc_dir)-1] != '/') {
			strcat(exc_dir, "/");
		}
		strcat(exc_dir, args[0]);
		execl(exc_dir, args[0], (char*)NULL);	
	}
	printf("error: %s\n", strerror(errno));
	exit(EXIT_FAILURE);			
}

void shell_record(char *line) 
{
	insert(&history, line);
	if (size(&history) > 100) {
		delete_first(&history);
	}
}

void print_history() 
{
	int i = 0;
	Node *p = history.head;
	while ((p = p->next) != history.tail) {
		printf("%d %s\n", i++, p->str);
	}
}

void shell_history(char **args, int num)
{
	int i;
	int his_num;
	Node *p;
	delete_last(&history);
	if (num == 1) {
		print_history();
		return;
	}
	if (!strcmp(args[1], "-c")) {
		clear(&history);
		Init_link(&history);
		return; 
	}
	for (i = 0; i < strlen(args[1]); ++i) {
		if(args[1][i] > '9' || args[1][i] < '0') {
			printf("error: %s\n", "invalid history number");
			return;
		}
	}
	his_num = atoi(args[1]);
	if (his_num <= 0 || his_num > size(&history)) {
		printf("error: %s\n", "invalid history number");
		return;		
	}
	p = history.head;
	while ((p = p->next) != history.tail) {
		if (--his_num <= 0) {
			printf("%s\n", p->str);
			return;
		}			
	}		
}

void print_paths() 
{
	Node *p = paths.head->next;
	if (p == paths.tail) {
		printf("\n");
		return; 
	}
	printf("%s", p->str);
	while ((p = p->next) != paths.tail) {
		printf(":%s", p->str);
	}
	printf("\n");
}

void shell_path(char **args, int num) 
{
	if (num == 1) {
		print_paths();
		return;
	}
	if (num == 2) {
		return;	
	}
	if (!strcmp(args[1], "+")) {		
		insert(&paths, args[2]);
	}	
	else if (!strcmp(args[1], "-")) {
		delete_str(&paths, args[2]);						
	}
}

void shell_pwd(int num) 
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, PATH_MAX) == NULL) {
		printf("error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);			
	} 
	else {
		printf("%s\n", cwd);	
	}
}

void shell_cd(char **args, int num)
{
	if (num < 2)
		return;
	if (chdir(args[1]) != 0) {
		printf("error: %s\n", strerror(errno));
		return;			
	}
}

#define MAX_ARG 64
char **parse_command(char *line, int *num, char *delim)
{
	char *arg;
	char **args = malloc(MAX_ARG * sizeof(char*));
	if (args == NULL) {
		printf("error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);	
	}
	*num = 0;
	for (arg = strtok(line, delim); arg != NULL; arg = strtok(NULL, delim)) {
		args[(*num)++] = arg;
		if ((*num) >= MAX_ARG) {
			printf("error: %s\n", "Excess maximum arguments");
			exit(EXIT_FAILURE);					
		}	
	}
	return args;
}

char *read_line() 
{
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, stdin) == -1) {
		printf("error: %s\n", "Cannot read a line");
		exit(EXIT_FAILURE);
	}
	len = strlen(line);
	if (line[len-1] == EOF || line[len-1] == '\n')
		line[len-1] = '\0';		
	return line;
}


