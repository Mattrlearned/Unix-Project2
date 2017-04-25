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


int mysh_execWithRD(char** args, char* inFile, char* outFile, int in, int out){
    int status;
    int ofd;
    int ifd;
    pid_t pid, wpid;
    
    pid = fork();
    if (pid == 0)
    {
        if(in == 1){
	ifd = open(inFile,O_RDONLY);
	dup2(ifd,STDIN_FILENO);
	close(ifd);
        }
        if(out == 1){
	ofd = open(outFile,O_WRONLY|O_CREAT | O_TRUNC, S_IRUSR |S_IRGRP | S_IWGRP | S_IWUSR); 
	dup2(ofd,STDOUT_FILENO);
	close(ofd);
        }
	char *progPath = getenv(args[0]);
	if (!progPath) {
		progPath = args[0];
	}
	if (execvp(progPath, args) == -1) {
		printf("fail\n");
		exit(EXIT_FAILURE);
	}
        exit(1);
      	} else if( pid < 0){ exit(1);}
        else {
		do {
                        wpid = waitpid(pid, &status, WUNTRACED);
       } while (!WIFEXITED(status) && !WIFSIGNALED(status));
       }
   fflush(stdout);
   fflush(stdin);  
   return 1;
}
   
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

int main(int argc, char const *argv[]) {
	const int SIZE = 512;
	int status;
	char line[SIZE];
	char temp_buf[SIZE];
	char **args = NULL;
	char **pipes = NULL;

	do {
		set_env_variables();
                char* color1 = "\033[36m";
                char* color2 = "\033[0m";
                char* prompt = "mysh-$ ";
                printf("%s%s%s",color1,  prompt, color2);
		fgets(line, SIZE, stdin);
		line[strcspn(line, "\n")] = 0; //remove newline character

		/*check for pipes */
		if (strstr(line, "|")) {
			splitLine(strcpy(temp_buf, line), &pipes, "|"); //split pipe cmds
			status = mysh_execPipe(pipes);
		} else if (strstr(line, "$(")) {
			char* cmd = procSubshells(line);
			status = execSubshell(cmd);
		}  
                else if(strstr(line, ">") || strstr(line, "<")){
                	char *inFile = NULL;
           		char *outFile = NULL;
                         int in =1;
                         int out =1;
          		 getRedir(strcpy(temp_buf, line), &args, " ", &inFile, &outFile, &in, &out);
          		 status =  mysh_execWithRD(args, inFile, outFile, in, out);
 		}
                else {
			splitLine(strcpy(temp_buf, line), &args, " "); //split into tokens

			if (strcmp(args[0], "mypwd") == 0) {
				status = mypwd();
			} else if (strcmp(args[0], "mycd") == 0) {
				status = mycd(args);
			}

			else if (strcmp(args[0], "exit") == 0|| strcmp(args[0], "quit") == 0)
				status = 0;
			else
				status = mysh_execute(args);
		}
	} while (status);

	free(args);
	free(pipes);
	return EXIT_SUCCESS;
}


