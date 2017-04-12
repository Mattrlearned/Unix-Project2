#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void clearArr(char **argv)
{
    int i = 0;

    while(argv[i] != NULL)
    {
        argv[i] = NULL;
        i++;
    }
}
char *parseFileName(char **argv, char *fileName)
{
    int i = 0;

    while(strcmp(argv[i], "<") != 0 && strcmp(argv[i], ">") != 0)
    {
        i++;
    }

    //found <>
    argv[i++] = NULL;
    strcpy(fileName, argv[i]);
    argv[i] = NULL;

}
void copyUntilPipe(char **argv, char **subArgv, int *argvPtr)
{
    int i = *argvPtr;
    int j = 0;

    while(argv[i] != NULL && strcmp(argv[i], "|") != 0)
    {
        subArgv[j] = argv[i];
        i++;
        j++;
    }

    subArgv[j] = NULL;
    //argv[i] is pointing to |
    i++;

    *argvPtr = i;
}

void printHistory(char history[30][256], int *hIndex)
{
    printf("\n");
    int i = *hIndex;
    for(i = 0; strcmp(history[i], "NULL") != 0 && i < 30; ++i)
    {
        printf("%s\n", history[i]);
    }

    printf("\n");
}

int findInRedir(char **argv, int argc)
{
    int i = 0;

    for(i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "<") == 0)
        {
            return 1;
        }
    }

    return 0;
}

int findOutRedir(char **argv, int argc)
{
    int i = 0;

    for(i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], ">") == 0)
        {
            return 1;
        }
    }

    return 0;
}

int run(char **args, int prevPipe, int isFirst, int isLast, int inRedir, int outRedir, char *fileName)
{
    //fd[0] is output to pipe, fd[1] is input from pipe
    int fd[2];
    pipe(fd);
    //fork child to run command

    int pid = fork();

    if(pid == 0)
    {
        if(isFirst == 1 && isLast == 1 && inRedir == 1) //one process w/ redir
        {  
            int f = open(fileName, O_RDONLY);
            close(0);
            dup(f);
        }
        else if(isFirst == 1 && inRedir == 1) //first process and needs redir
        {   
            //setup input from file
            int f = open(fileName, O_RDONLY);
            close(0);
            dup(f);
            
            //setup output to pipe
            close(1);
            dup(fd[1]);
        }
        else if(isLast == 1 && outRedir == 1) //last process and needs redirection
        {
            //output to file
            fflush(stdout);
            int f = open(fileName, O_CREAT|O_WRONLY, 0777);
            close(0);
            dup(prevPipe);
            close(1);
            dup(f);   
            //input from pipe
            close(f);
        }
        else if(isLast == 1)
        {

            //setup input from pipe
            close(0);
            dup(prevPipe);
            close(fd[1]);            
        }
        else if(isFirst == 1)
        {
            //setup output to pipe          
            close(1);
            dup(fd[1]);
        }
        else //middle process
        {
            //read from pipe
            close(0);
            dup(prevPipe);
            //setup pipe output
            close(1);
            dup(fd[1]);
            //execute the middle process              
        }
        //execute the command
        if(execvp(args[0], args) == -1)
        {
            exit(0);
        }

    }
    wait(NULL);

    
    close(fd[1]);

    if (isLast == 1)
    {
        close(fd[0]);
    }
    //need to return pipe end to be read from
   // printf("RETURNING FD %d", fd[0]);
    return fd[0];
}

int findAmpersand(char **argv, int count)
{
    int i = 0;

    for(i = 0; i < count; ++i)
    {
        if(strcmp(argv[i], "&") == 0)
        {
            return 1;
        }
    }

    return 0;
}

void copyToHistory(char *str, char history[30][256], int *hIndex)
{
    int i = 0;
    int flag = 0;


    strcpy(history[*hIndex], str);
    *hIndex = (*hIndex + 1) % 30;

}

int findPipes(char **argv, int argc)
{
    int count = 0;
    int i = 0;

    for(i = 0; i < argc; ++i)
    {
        if(strcmp("|", argv[i]) == 0)
        {

            count++;

        }
    }

    return count;
}

int tokenize(char *line, char **argv)
{
    int count = 0;

    while(*line != '\0') 
    {       
        while(*line == ' ' || *line == '\n') //strip white spaces (replace with \0)
        {
            *line = '\0';
            line++;
        }

        *argv = line;
        argv++;
        count++;

        while(*line != '\0' && *line != ' ' && *line != '\n') //skip characters until next delimiter
        {
            line++;
        }            
     }

     *argv = '\0'; //add null terminator to argv
     return count;
}

void clearHistory(char history[30][256])
{
    int i = 0;
    for(i = 0; i < 30; ++i)
    {
        strcpy(history[i], "NULL");
    }
}

int main()
{
    int hIndex = 0;
    char *argv[256];
    char *subArgv[256];
    char history[30][256];
    int runs = 0;

    clearHistory(history);

    while(1)
    {
        char fileName[256];
        int argvPtr = 0;
        char str[256];
        printf("sssh> ");
        gets(str); //grab string (pls dont murder me for using gets)
        copyToHistory(str, history, &hIndex); //copy string to history
        int count = tokenize(str, argv); //tokenize string (have argv point to words in string)+
        
        int pipeCount = findPipes(argv, count);
        int inRedir = findInRedir(argv, count);
        int outRedir = findOutRedir(argv, count);
        int hasAmpersand = findAmpersand(argv, count);
        int isFirst = 1;
        int isLast = 0;
        int prevPipe = 0;

        
        if(strcmp(argv[0],"quit") == 0 ) exit (0);

        else if(strcmp(argv[0], "history") == 0)
        {
            printHistory(history, &hIndex);
        }

        else if(strcmp(argv[0], "echo") == 0)
        {

            int i = 0;
            for(i = 1; i < count; ++i)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
        }
        else if(strcmp(argv[0], "cd") == 0)
        {
            chdir(argv[1]);
        }
        else if(hasAmpersand == 1)
        {
            printf("Operation not supported: please enter a command without an ampersand operator\n");
        }
        else
        {
            if(pipeCount == 0) //one command
            {
                isLast = 1;
                copyUntilPipe(argv,subArgv, &argvPtr);

                if(inRedir)
                {
                    parseFileName(subArgv, fileName);
                    run(subArgv, prevPipe, isFirst, isLast, inRedir, outRedir, fileName);
                }
                else
                {
                    run(subArgv, prevPipe, isFirst, isLast, inRedir, outRedir, fileName);
                }
            }
            else
            {
                int i = 0;
            
                for(i = 0; i < pipeCount; ++i)
                {
                    copyUntilPipe(argv, subArgv, &argvPtr);

                    if(inRedir == 1 && i == 0)
                    {
                        parseFileName(subArgv, fileName);
                    }
                    prevPipe = run(subArgv, prevPipe, isFirst, isLast, inRedir, outRedir, fileName);

                    clearArr(subArgv);
                    isFirst = 0; 
                }
                //run last command
                //int run(char **args, int prevPipe, int isFirst, int isLast, int inRedir, int outRedir, char *fileName)
                isLast = 1;
                copyUntilPipe(argv, subArgv, &argvPtr);

                if(outRedir == 1)
                {
                    parseFileName(subArgv, fileName);
                }
                prevPipe = run(subArgv, prevPipe, isFirst, isLast, inRedir, outRedir, fileName);
            }
        }
            //done and clear stuff
        clearArr(argv);
        clearArr(subArgv);

    }

 }

    


    
