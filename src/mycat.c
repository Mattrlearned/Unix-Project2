#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
   if (argc == 1){ //input is from stdin, not a file
	int bufSize = 512;
	char* str = (char*) malloc(bufSize);
	int isInput;
	while(1){
	   isInput = getline(&str, &bufSize, stdin);
	   if(isInput == -1) break;
	   printf("%s", str);
	}
	free(str);
	return 1;
   }
   else{ //reading from a file
      char c;
      FILE *file;
      struct stat fStat;
      file = fopen(argv[1],"r");

      if(file == NULL){
         warn("%s", argv[1]);
         return 0;
      }
      else if(file != NULL && stat(argv[1], &fStat) <0){
         warn("%s", argv[1]);
         return 0;
      }
      else if(S_ISDIR(fStat.st_mode)){
         fprintf(stderr, "mycat: %s: Is a directory\n", argv[1]);
         return 0;
      }
      else {
         while((c = getc(file)) != EOF)
            putchar(c);
         fclose(file);
      }
   }
}                       
