#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



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

char** splitLine(char* line) {
  char** cmdStr = NULL;
  char* tok = strtok(line, " ");
  int i = 0;
  
  while (tok) {
    cmdStr = realloc(cmdStr, sizeof(char*) * i++);
    cmdStr[i - 1] = tok;
    tok = strtok(NULL, " ");
  }
  
  cmdStr = realloc(cmdStr, sizeof(char*) * (i+1));
  cmdStr[i] = 0;    
  return cmdStr;
}

int mycd(char **args){
    //error check
    if (args[1] == NULL) //args[1] is target directory 
       fprintf(stderr, "mycd expects an argument \n");
  //add mycd code here
}

//setting environment variables for non-builtin cmds
void set_env_variables(){
   char buffer[100];
   realpath("myls",buffer);
   setenv("myls",buffer, 0);
   realpath("mycat", buffer);
   setenv("mycat", buffer, 0);

}

int main(int argc, char const *argv[]) {
  const int SIZE = 512;
  int status;
  char line[SIZE];
  char temp_buf[SIZE];

  do {
    set_env_variables();
    printf("mysh-$ ");

    fgets(line, SIZE, stdin);
    line[strcspn(line, "\n")] = 0; //remove newline character
    char** args=splitLine(strcpy(temp_buf, line)); //split into tokens

    // check if cmd is builtin
    if (strcmp(args[0], "mypwd") == 0) {
       printf("%s\n", getenv("PWD"));
    }
    else if (strcmp(args[0], "mycd") == 0) {
       status = mycd(args);        
    }

    else if(strcmp(args[0], "exit") == 0)
       status=0;
    else
       status = mysh_execute(args);
  } while (status);

  return EXIT_SUCCESS;
}



