#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MYSH_TOK_BUFSIZE 64
#define MYSH_TOK_DELIMITER " \t\r\n\a"

int mysh_cd(char **args);
int mysh_help(char **args);
int mysh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &mysh_cd,
  &mysh_help,
  &mysh_exit
};

int mysh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int mysh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mysh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int mysh_help(char **args)
{
  int i;
  printf("GU'S UNIX PROJECT 2\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < mysh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int mysh_exit(char **args)
{
  return 0;
}

int mysh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("mysh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int mysh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < mysh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return mysh_launch(args);
}

char* mysh_read()
{
	char* line = NULL;
	size_t bufSize = 0;
	getline(&line, &bufSize, stdin);
	return line;
}

char** mysh_parse(char* line)
{
	int bufSize = MYSH_TOK_BUFSIZE;
	int pos = 0;
	char** tokens = malloc(bufSize * sizeof(char*));
	char* token;
   
   	token = strtok(line, MYSH_TOK_DELIMITER);
    while (token != NULL) {
    	tokens[pos] = token;
    	pos++;
    	if (pos >= bufSize)
    	{
    		bufSize += MYSH_TOK_BUFSIZE;
    		tokens = realloc(tokens, bufSize * sizeof(char*));
    		if (!tokens) 
    		{
    			fprintf(stderr, "mysh: memory allocation error\n");
    			exit(EXIT_FAILURE);
    		}
    	}
    	token = strtok(NULL, MYSH_TOK_DELIMITER);
    }
    tokens[pos] = NULL;
    return tokens;
}

void mysh_loop()
{
	char *line;
	char **args;
	int status;

	do {
		printf("$ \n");
		line = mysh_read();
		args = mysh_parse(line);
		status = mysh_execute(args);
		free(line);
		free(args);
	} while (status);
}

int main(int argc, char const *argv[])
{
	// Run command loop
	mysh_loop();
	return EXIT_SUCCESS;
}