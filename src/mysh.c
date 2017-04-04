#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int mysh_execute(const char* line) {
  pid_t pid;
  int status;

  if (line == NULL) {
    return(0);
  }

  pid = fork();
  if (pid == 0) {
    execl("/bin/sh", "sh", "-c", line, (char *)0);      
    exit(pid);
  } 
  else if (pid < 0) {
    printf("Error, fork\n");
  } 
  else {
    wait(NULL);
  }

  return 1;
}

int main(int argc, char const *argv[]) {
  int status;
  size_t buf = 0;
  char* line = NULL;

  do {
    printf("mysh-$ ");
    getline(&line, &buf, stdin);
    status = mysh_execute(line);
  } while (status);

  free(line);

  return EXIT_SUCCESS;
}



