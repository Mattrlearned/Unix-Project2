#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void removeChar(char *str, char toRemove) {
    char *old, *new;
    for (old = new = str; *old != '\0'; old++) {
        *new = *old;
        if (*new != toRemove) new++;
    }
    *new = '\0';
}

char *appendFileToBuf(char *argv, char *filePath)
{
    FILE *f = fopen("temp.txt", "r");
    char *fBuf;
    int count = 0;
     
    printf("%d", errno);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    
    clearerr(f);
    int strSize = strlen(argv);
    fBuf = malloc((size + 1)); //need to hold file + terminator
    //rewind and read file
    fseek(f, 0, SEEK_SET);
    //read file
    int i = 0;
    while(1)
    {
        char c = fgetc(f);
        if(feof(f))
        {
            break;
        }
        fBuf[i] = c;
        i++;
    }
    fBuf[size] = '\0';

    char *buf = malloc(strSize + size + 1); 
    buf[0] = '\0';

    strcat(buf, argv);
    strcat(buf, fBuf);
    free(fBuf);
    //point last element of argve to NULL
    fclose(f);

    return buf;
}
 
int mysh_execute(char** args) {
	pid_t pid, wpid;
	int status;
	pid = fork();
	if (pid == 0) {
		char *progPath = getenv(args[0]);
		if (!progPath) {progPath = args[0];}
		if (execvp(progPath, args) == -1) {
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (pid < 0) {
		printf("Error, fork\n");
	}
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int numDelims(char* str, char* delim) {
	int num = 0;
	char* c;
	for (c = delim; *delim; ++delim) {
		while (*str) {
			if (*str == *c)
				num++;
			str++;
		}
	}
	return num;
}

void splitLine(char* line, char*** cmdStr, char* delims) {
	*cmdStr = malloc(sizeof(char*) * (numDelims(line, delims) + 2));
	if (*cmdStr) {
		int i = 0;
		char* token = strtok(line, delims);
		while (token != NULL) {
                   if(!strstr(token, "|")){                        
		      (*cmdStr)[i++] = strdup(token);
                   }
		   token = strtok(NULL, delims);
		}
		(*cmdStr)[i] = '\0';
	}
}


void tokenize(char *str, char ***argv)
{
    int i = 0, count = 0;
    while(1)
    {
        char c = str[i];
        if(c == '\0')
        {
            break;
        }
        if(c == '\n' || c == ' ')
        {
            count++;
        }
        i++;
    }
    //point argv to buf
    *argv = malloc(sizeof(char *) * (count + 1));
    //tokenize and replace newlines with null terminators
    i = 0;
    while(*str != '\0')
    {
        if(*str == '\n' || *str == ' ')
        {
            //replace newline with null
            *str = '\0';
            str++;
        }
        //point argve to next line
        (*argv)[i] = str;
        i++;
        while(*str != '\n' && *str != '\0' && *str != ' ')
        {
            str++;
        }
    }
    //point last element of argve to NULL
    (*argv)[count] = '\0';
}


int mysh_execSubshell(char **pipes)
{
    //need to execute everything between $()
    char **args1 = NULL;
    char **args2 = NULL;
    splitLine(pipes[1], &args2, " ");

    pid_t pid;
    int status;
    int fd = open("temp.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    pid = fork();
    //run the cmd inside subshell
    if(pid == 0)
    {
        close(1);
        dup(fd);
        char *progPath = getenv(pipes[1]);
        if (!progPath) {progPath = pipes[1];}
        if(execvp(progPath, args2) == -1)
        {
            printf("couldn't exec somehow\n");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
        fflush(stdin);
        exit(1);
    }

    fsync(fd);
    wait(0); //child needs to finish before parent can fork second child
    pid = fork();
    if(pid == 0)
    {
        char *progPath = getenv(pipes[0]);
        if(!progPath) { progPath = pipes[0];}
        char *newArg = appendFileToBuf(pipes[0], "temp");
        //i had to call my own tokenize due to weird bug
        tokenize(newArg, &args1);
        if(execvp(args1[0], args1) == -1)
        {
            printf("couldn't exec for some reason\n");
            exit(EXIT_FAILURE);
        }
        exit(1);
    } 

    wait(0);
    unlink("temp.txt");
    free(args1);
    free(args2);

    return 1;

}

 
int mysh_execPipe(char** pipes){
	char** args1 = NULL;
	char** args2 = NULL;
	splitLine(pipes[0],&args1, " ");
        splitLine(pipes[1],&args2, " ");	
        pid_t pid1, pid2;
	int fd[2];
	pipe(fd);

	pid1=fork();
	if (pid1 == 0) {
		dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
		close(fd[STDIN_FILENO]);
		close(fd[STDOUT_FILENO]);
		char *progPath = getenv(args1[0]);
		if (!progPath) {progPath = args1[0];}
		if (execvp(progPath, args1) == -1){
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
		exit(1);
	}
	pid2=fork();
	if (pid2 == 0) {
		dup2(fd[STDIN_FILENO], STDIN_FILENO);
		close(fd[STDOUT_FILENO]);
		close(fd[STDIN_FILENO]);
		char *progPath = getenv(args2[0]);
		if (!progPath) {progPath = args2[0];}
		if(execvp(progPath, args2) == -1){
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

void subshells(char* line, char*** cmdStr) {
  char* delim = "$";
  int i =0;
  removeChar(line, '(');
  removeChar(line, ')');

  *cmdStr = malloc(sizeof(char*) * (numDelims(line, delim) + 2));
  char* token = strtok(line, delim);
  while (token != NULL) {
      (*cmdStr)[i++] = strdup(token);
      token = strtok(0, delim);
  }
  (*cmdStr)[i] = '\0';
}


int mycd(char **args){
	if (args[1] == NULL) //args[1] is target directory
		fprintf(stderr, "Error: Mycd expects an argument \n");
	else if (chdir(args[1]) == -1){
		fprintf(stderr, "Error: Directory could not be found\n");
	}
	return 1;
}

int mypwd(){
	char *currentPath = malloc(1024);
	if(getcwd(currentPath, 1024) == '\0') {
		perror("getcwd");
	}
	printf("%s\n", currentPath);
	return 1;
}

void set_env_variables(){
	char buffer[100];
	realpath("myls",buffer);
	setenv("myls",buffer, 0);
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
		printf("mysh-$ ");

		fgets(line, SIZE, stdin);
		line[strcspn(line, "\n")] = 0; //remove newline character

		/*check for pipes */
		if(strstr(line, "|")){
			splitLine(strcpy(temp_buf, line), &pipes, "|"); //split pipe cmds
			status = mysh_execPipe(pipes);
		}
                else if(strstr(line, "$(")){
                     subshells(strcpy(temp_buf, line), &pipes); //this removes '(', ')' and splits on '$'
                     status = mysh_execSubshell(pipes);
                     //we will need to redirect output of pipe[1] to input of pipe[2]
	        }		
		else{
			splitLine(strcpy(temp_buf, line), &args, " "); //split into tokens

			/* check if cmd is a built-in or exit*/
			if (strcmp(args[0], "mypwd") == 0) {
				status = mypwd();
			}
			else if (strcmp(args[0], "mycd") == 0) {
				status = mycd(args);
			}

			else if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0)
				status=0;
			else
				status = mysh_execute(args);
		}
	} while (status);

	free(args);
	free(pipes);
	return EXIT_SUCCESS;
}

