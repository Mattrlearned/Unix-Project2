#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>     

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
      file = fopen(argv[1],"r");
      if(file) {
         while((c = getc(file)) != EOF)
            putchar(c);
         fclose(file);
      }
      else {
         fprintf(stderr, "File not found.\n");
         exit(1);
      }
   }
}                       
