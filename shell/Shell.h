#ifndef SHELL_H_
#define SHELL_H_

char *read_line(void);
char **parse_command(char *line, int *num, char *delim);
void shell_pwd(int num);
void shell_cd(char **args, int num);
void print_paths(void);
void shell_path(char **args, int num);
void search(char **args, int num);
int shell_execute(char **args, int num);
void print_history(void);
void shell_history(char **args, int num);
int shell_cmd(char *line);
void execute_his(const char *str);

#endif
