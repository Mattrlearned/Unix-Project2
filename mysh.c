#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "textProc.h"
#include "subshell.h"

int mysh_execute(char** args) {
	pid_t pid, wpid;
	int status;
	pid = fork();
	if (pid == 0) {
		char *progPath = getenv(args[0]);
		if (!progPath) {
			progPath = args[0];
		}
		if (execvp(progPath, args) == -1) {
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
	} else if (pid < 0) {
		printf("Error, fork\n");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int mysh_execPipe(char** pipes) {
	char** args1 = NULL;
	char** args2 = NULL;
	splitLine(pipes[0], &args1, " ");
	splitLine(pipes[1], &args2, " ");
	pid_t pid1, pid2;
	int fd[2];
	pipe(fd);

	pid1 = fork();
	if (pid1 == 0) {
		dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
		close(fd[STDIN_FILENO]);
		close(fd[STDOUT_FILENO]);
		char *progPath = getenv(args1[0]);
		if (!progPath) {
			progPath = args1[0];
		}
		if (execvp(progPath, args1) == -1) {
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
		exit(1);
	}
	pid2 = fork();
	if (pid2 == 0) {
		dup2(fd[STDIN_FILENO], STDIN_FILENO);
		close(fd[STDOUT_FILENO]);
		close(fd[STDIN_FILENO]);
		char *progPath = getenv(args2[0]);
		if (!progPath) {
			progPath = args2[0];
		}
		if (execvp(progPath, args2) == -1) {
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
		exit(1);
	}
	close(fd[STDIN_FILENO]);
	close(fd[STDOUT_FILENO]);
	wait(0);
	wait(0);

	free(args1);
	free(args2);
	fflush(stdout);
	fflush(stdin);
	return 1;
}

int mycd(char **args) {
	if (args[1] == NULL) //args[1] is target directory
		fprintf(stderr, "Error: Mycd expects an argument \n");
	else if (chdir(args[1]) == -1) {
		fprintf(stderr, "Error: Directory could not be found\n");
	}
	return 1;
}

int mypwd() {
	char *currentPath = malloc(1024);
	if (getcwd(currentPath, 1024) == '\0') {
		perror("getcwd");
	}
	printf("%s\n", currentPath);
	return 1;
}

void set_env_variables() {
	char buffer[100];
	realpath("myls", buffer);
	setenv("myls", buffer, 0);
	realpath("mycat", buffer);
	setenv("mycat", buffer, 0);
	realpath("mycp", buffer);
	setenv("mycp", buffer, 0);

}
int my_redirec(char** args) {
  	int add_to_command = 1;
	int bufsize = 256;
	int in = 0;
  	int out = 0;
  	pid_t pid; 
  	char** command = malloc(bufsize * sizeof(char**));
  	for (int i = 0; args[i] != NULL; i++) {
    		if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
      			if (add_to_command) {
        		add_to_command = 0;
        		command[i] = NULL;
      		}
      		int status;
      		pid = fork();
		if (pid == 0) {
        		if (strcmp(args[i], "<") == 0) {
          			in = open(args[i + 1], O_RDONLY);
          			dup2(in, 0);
          			close(in);
        		} else if (strcmp(args[i], ">") == 0) {
          			out = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR |S_IRGRP | S_IWGRP | S_IWUSR);
          			dup2(out, 1);
          			close(out);
        		}
			char *progPath = getenv(command[0]);
			if (!progPath) {
				progPath = command[0];
			}
			if (execvp(progPath, command) == -1) {
				printf("fail\n");
				exit(EXIT_FAILURE);
			}
        		exit(1);
      		} else {
        		while (wait(&status) != pid);
      			}
    		}
    		if (add_to_command) {
      			command[i] = args[i];
    		}
  	}
  return 1;
}


/* char* getCurrentDirectory() {
 	char* currentPath = malloc(1024);
 	char* currentDirectory;
 	if(getcwd(currentPath, 1024) == '\0') {
 		perror("getcwd");
 	}

 	currentDirectory = malloc(strlen(currentPath) + 1);

 	int done = 0;
 	int i = strlen(currentPath) + 1;

 	while(!done) {
 		if (currentPath[i] == '/')
 		{
 			done = 1;
 			i++;
 		}
 		else {
 			i--;
 		}
 	}

     strcpy(currentDirectory, &currentPath[i]);

 	return currentDirectory;
 }

 char* getHost() {
 	char* hostname = malloc(148);
 	int status;
 	status = gethostname(hostname, sizeof(hostname)+1);
 	if (status != 0)
 		return NULL;
 	else {
 		for (int i = 0; i < strlen(hostname); ++i)
 		{
 			if (hostname[i] == '.') {
 				hostname[i] = '\0';
 			}
 		}
 	return hostname;
 	}
 } */

int main(int argc, char const *argv[]) {
	const int SIZE = 512;
	int status;
	char line[SIZE];
	char temp_buf[SIZE];
	char **args = NULL;
	char **pipes = NULL;

	/* Get uid & current directory
	register struct passwd *p;
	register uid_t uid;
	uid = geteuid();
	p = getpwuid(uid);
	char* currentDirectory = getCurrentDirectory();
	char* hostname = getHost(); */

	do {
		set_env_variables();
		// currentDirectory = getCurrentDirectory();
		// if (p)
		// 	printf("%s-mysh:%s %s$ ", hostname, currentDirectory, p->pw_name);
		// else
		// 	printf("mysh-$ ");
		printf("mysh-$ ");
		fgets(line, SIZE, stdin);
		line[strcspn(line, "\n")] = 0; //remove newline character

		/*check for pipes */
		if (strstr(line, "|")) {
			splitLine(strcpy(temp_buf, line), &pipes, "|"); //split pipe cmds
			status = mysh_execPipe(pipes);
		} else if (strstr(line, "$(")) {
			char* cmd = procSubshells(line);
			status = execSubshell(cmd);
		} else {
			splitLine(strcpy(temp_buf, line), &args, " "); //split into tokens

			if (strcmp(args[0], "mypwd") == 0) {
				status = mypwd();
			} else if (strcmp(args[0], "mycd") == 0) {
				status = mycd(args);
			}

			else if (strcmp(args[0], "exit") == 0
					|| strcmp(args[0], "quit") == 0)
				status = 0;
			else if (strstr(line, "<")||strstr(line,">")) {
				status = my_redirec(args);
			}
			else
				status = mysh_execute(args);
		}
	} while (status);

	free(args);
	free(pipes);
	return EXIT_SUCCESS;
}

