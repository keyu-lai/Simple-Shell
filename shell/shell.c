#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "Link.h"
#include "Shell.h"

struct Link paths;
struct Link history;

int main(int argc, char **argv)
{
	char *line;

	Init_link(&paths);
	Init_link(&history);

	while (1) {
		printf("$");
		line = read_line();
		if (shell_cmd(line))
			break;
	}

	clear(&paths);
	clear(&history);
	return EXIT_SUCCESS;
}

#define EXIT_LOOP 1
#define CONTINUE_LOOP 0
int shell_cmd(char *line)
{
	int num;
	char **args;
	int res = CONTINUE_LOOP;

	insert(&history, line);
	args = parse_command(line, &num, " ");
	if (num == 0)
		delete_last(&history);
	else if (!strcmp(args[0], "exit"))
		res = EXIT_LOOP;
	else if (!strcmp(args[0], "cd"))
		shell_cd(args, num);
	else if (!strcmp(args[0], "pwd"))
		shell_pwd(num);
	else if (!strcmp(args[0], "path"))
		shell_path(args, num);
	else if (!strcmp(args[0], "history"))
		shell_history(args, num);
	else
		res = shell_execute(args, num);
	free(line);
	free(args);
	return res;
}

int shell_execute(char **args, int num)
{
	int pid;

	pid = fork();
	if (pid < 0) {
		printf("error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		search(args, num);
		return EXIT_LOOP;
	} else {
		int return_code;

		while (pid != wait(&return_code)) {}
		return CONTINUE_LOOP;
	}
}

void search(char **args, int num)
{
	char *exc_dir;
	struct Node *p = paths.head;

	if (index(args[0], '/') != NULL) {
		execv(args[0], args);
		printf("error: %s\n", strerror(errno));
		return;
	}
	if (size(&paths) == 0) {
		printf("error: %s\n", "No such file or directory");
		return;
	}
	while ((p = p->next) != paths.tail) {
		int dir_len = strlen(p->str) + strlen(args[0]) + 2;

		exc_dir = malloc(dir_len * sizeof(char));
		if (exc_dir == NULL)
			malloc_failure();
		strcpy(exc_dir, p->str);
		if (exc_dir[strlen(exc_dir)-1] != '/')
			strcat(exc_dir, "/");
		strcat(exc_dir, args[0]);
		execv(exc_dir, args);
		free(exc_dir);
	}
	printf("error: %s\n", strerror(errno));
}

#define HISTORY_MAX 100
void print_history(void)
{
	int i = 0;
	struct Node *p = history.head;

	while (size(&history) > HISTORY_MAX)
		delete_first(&history);
	while ((p = p->next) != history.tail)
		printf("%d %s\n", i++, p->str);
}

void execute_his(const char *str)
{
	char *line;

	line = malloc((strlen(str) + 1) * sizeof(char));
	if (line == NULL)
		malloc_failure();
	strcpy(line, str);
	shell_cmd(line);
}

void shell_history(char **args, int num)
{
	int i;
	int his_num;
	struct Node *p;

	delete_last(&history);
	if (num == 1) {
		print_history();
		return;
	}
	if (num > 2) {
		printf("error: %s\n", "Invalid history command");
		return;
	}
	if (!strcmp(args[1], "-c")) {
		clear(&history);
		Init_link(&history);
		return;
	}

	for (i = 0; i < strlen(args[1]); ++i) {
		if (args[1][i] > '9' || args[1][i] < '0') {
			printf("error: %s\n", "Invalid history command");
			return;
		}
	}
	his_num = atoi(args[1]);
	if (his_num >= size(&history) || his_num >= HISTORY_MAX) {
		printf("error: %s\n", "Invalid history command");
		return;
	}

	p = history.head;
	while ((p = p->next) != history.tail) {
		if (--his_num < 0) {
			execute_his(p->str);
			return;
		}
	}
}

void print_paths(void)
{
	struct Node *p = paths.head->next;

	if (p == paths.tail) {
		printf("\n");
		return;
	}
	printf("%s", p->str);
	while ((p = p->next) != paths.tail)
		printf(":%s", p->str);
	printf("\n");
}

void shell_path(char **args, int num)
{
	if (num == 1) {
		print_paths();
		return;
	}
	if (num == 2 || num > 3) {
		printf("error: %s\n", "Invalid path command");
		return;
	}

	if (!strcmp(args[1], "+")) {
		if (!check_duplicate(&paths, args[2]))
			insert(&paths, args[2]);
	} else if (!strcmp(args[1], "-"))
		delete_str(&paths, args[2]);
	else
		printf("error: %s\n", "Invalid path command");
}

void shell_pwd(int num)
{
	char cwd[PATH_MAX];

	if (num > 1) {
		printf("error: %s\n", "Invalid pwd command");
		return;
	}

	if (getcwd(cwd, PATH_MAX) == NULL) {
		printf("error: %s\n", strerror(errno));
		return;
	}
	printf("%s\n", cwd);
}

void shell_cd(char **args, int num)
{
	if (num != 2) {
		printf("error: %s\n", "Invalid cd command");
		return;
	}
	if (chdir(args[1]) != 0) {
		printf("error: %s\n", strerror(errno));
		return;
	}
}

#define MAX_ARG 128
char **parse_command(char *line, int *num, char *delim)
{
	char *arg;
	char **args = malloc(MAX_ARG * sizeof(char *));

	if (args == NULL)
		malloc_failure();
	*num = 0;
	arg = strtok(line, delim);
	for (; arg != NULL; arg = strtok(NULL, delim)) {
		args[(*num)++] = arg;
		if ((*num) >= MAX_ARG-1) {
			printf("error: %s\n", "Excess maximum arguments");
			*num = 0;
			return args;
		}
	}
	return args;
}

char *read_line(void)
{
	char *line = NULL;
	size_t len = 0;

	if (getline(&line, &len, stdin) == -1) {
		printf("error: %s\n", "Cannot read a line");
		free(line);
		exit(EXIT_FAILURE);
	}
	len = strlen(line);
	if (line[len-1] == EOF || line[len-1] == '\n')
		line[len-1] = '\0';
	return line;
}


