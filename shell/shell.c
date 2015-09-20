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

/* Doubly linked list is used to store paths and history. */
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

/*
 * Function Used in while loop to parse a command.
 * If it returns EXIT_LOOP, the loop would break.
 * If it returns CONTINUE_LOOP, the loop would keep going.
 */
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

/*
 * Main function used to run a executable.
 * fork() is invoked. In the child process,
 * it uses search() to find the executable, then break the loop.
 * In the parent process, it just wait for the child to finish.
 */
int shell_execute(char **args, int num)
{
	int pid;
	int return_code;

	pid = fork();
	if (pid < 0) {
		printf("error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		search(args, num);
		return EXIT_LOOP;
	}
	while (pid != wait(&return_code))
		;
	return CONTINUE_LOOP;
}

/* Function used to search for the excutable in the paths we stored. */
void search(char **args, int num)
{
	char *exc_dir;
	struct Node *p = paths.head;

	/* If an executable has a path, we just run it using execv(). */
	if (index(args[0], '/') != NULL) {
		execv(args[0], args);
		printf("error: %s\n", strerror(errno));
		return;
	}
	if (size(&paths) == 0) {
		printf("error: %s\n", "No such file or directory");
		return;
	}
	/*
	 * If it don't have a path, we would search for it in paths.
	 * An iterative process is used to search in a linked list.
	 */
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

/* Main function for the history command. */
#define HISTORY_MAX 100
void shell_history(char **args, int num)
{
	int i;
	int his_num;
	struct Node *p;

	/* History commands don't need to be stored. */
	delete_last(&history);
	/* Print out the whole history. */
	if (num == 1) {
		print_history();
		return;
	}
	if (num > 2) {
		printf("error: %s\n", "Invalid history command");
		return;
	}
	/* Clear all history. */
	if (!strcmp(args[1], "-c")) {
		clear(&history);
		Init_link(&history);
		return;
	}

	/* Check if the history offset is numerical. */
	for (i = 0; i < strlen(args[1]); ++i) {
		if (args[1][i] > '9' || args[1][i] < '0') {
			printf("error: %s\n", "Invalid history command");
			return;
		}
	}
	/* Check if the history offset overflows. */
	his_num = atoi(args[1]);
	if (his_num >= size(&history) || his_num >= HISTORY_MAX) {
		printf("error: %s\n", "Invalid history command");
		return;
	}
	/* An iterative process is used to search a linked list. */
	p = history.head;
	while ((p = p->next) != history.tail) {
		if (--his_num < 0) {
			execute_his(p->str);
			return;
		}
	}
}

/* Function used to print out the whole hisotry. */
void print_history(void)
{
	int i = 0;
	struct Node *p = history.head;

	/* No more than 100 historie stored. */
	while (size(&history) > HISTORY_MAX)
		delete_first(&history);
	/* An iterative process is used to print out a linked list. */
	while ((p = p->next) != history.tail)
		printf("%d %s\n", i++, p->str);
}

/* Function used to execute a command stored in the hisotry. */
void execute_his(const char *str)
{
	char *line;

	/* line will be freed in the shell_cmd() function. */
	line = malloc((strlen(str) + 1) * sizeof(char));
	if (line == NULL)
		malloc_failure();
	strcpy(line, str);
	shell_cmd(line);
}

/* Main function for the path command. */
void shell_path(char **args, int num)
{
	/* Print out all the paths. */
	if (num == 1) {
		print_paths();
		return;
	}
	if (num == 2 || num > 3) {
		printf("error: %s\n", "Invalid path command");
		return;
	}

	if (!strcmp(args[1], "+")) {
		/* Relative path should not be stored. */
		if (args[2][0] == '/') {
			/* Check for duplicate. */
			if (!check_duplicate(&paths, args[2]))
				insert(&paths, args[2]);
		} else
			printf("error: %s\n", "Invalid path command");
	} else if (!strcmp(args[1], "-"))
		delete_str(&paths, args[2]);
	else
		printf("error: %s\n", "Invalid path command");
}

/* Function used to print out all the paths. */
void print_paths(void)
{
	struct Node *p = paths.head->next;

	if (p == paths.tail) {
		printf("\n");
		return;
	}
	/* An iterative process is used to print out a linked list. */
	printf("%s", p->str);
	while ((p = p->next) != paths.tail)
		printf(":%s", p->str);
	printf("\n");
}

/* Main function for the pwd command. */
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

/* Main function for the cd command. */
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

/*
 * Function used to extract tokens from a command line.
 * num is number of input arguments. It should not be more than 128.
 */
#define MAX_ARG 128
char **parse_command(char *line, int *num, char *delim)
{
	/* args will be freed in sehll_cmd(). */
	char *arg;
	char **args = malloc(MAX_ARG * sizeof(char *));

	if (args == NULL)
		malloc_failure();
	*num = 0;
	/* strtok() is used to extract tokens from strings. */
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

/* Read a line and store it. */
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


