#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#define BUFSIZE 1024
#define COPYBUF 0644

int getstat(char *filename, char* fileN)
{
    struct stat fileInfo;
    if ( (strcmp(fileN, ".") == 0) || (strcmp(fileN, "..") == 0) )
    	return -1;

    if(lstat(filename, &fileInfo) < 0) {
    	perror("Cannot open file.");
    	return -1;
    }
    
    if(!S_ISDIR(fileInfo.st_mode))
		return 1;
	else
		return 0;

    return -1;
}

int cpFile(char* src, char* dest) {
	
	// Check stat to verify src can be opened
	// If not, then handle as an error
	struct stat fileInfo;
	if (lstat(src, &fileInfo) < 0) {
		perror("Cannot open file");
	}

	char buf[BUFSIZE];
	int numChars;
	FILE* inFile;
	FILE* outFile;

	// Open source
	inFile = fopen(src, "r");
	if (inFile == NULL) {
		fprintf(stderr, "Error opening: %s\n", src);
		perror(src);
	} else
		printf("%s opened\n", src);

	// Create destination
	outFile = fopen(dest, "w");
	if (outFile == NULL) {
		fprintf(stderr, "Error creating: %s\n", dest);
		perror(dest);
	} else
		printf("%s created\n", dest);

	// Copy src to dest
	while( !feof(inFile) ) {
		numChars = fread(buf, 1, BUFSIZE, inFile);
		if ( fwrite(buf, 1, numChars, outFile) != numChars ) {
			fprintf(stderr, "Error writing to: %s\n", dest);
			perror(dest);
		}

		if (numChars == -1) {
			fprintf(stderr, "Error reading from: %s\n", src);
			perror(src);
		}
	}

	// Close files
	fclose(inFile);
	fclose(outFile);

	return 0;
}

int cpDir(char* src, char* dest) {

    DIR* dirPtr;
    struct dirent *dentry;
    dirPtr = opendir(src);
    int fileStatus;
    char tempDestDir[512];
    char tempSrcDir[512];
    char path[512];
    char srcFilePath[512];
    FILE *f1; //file to be copied
    FILE *f2; //copy of the file

    while ((dentry = readdir(dirPtr)) != NULL){
       strcpy(tempSrcDir, src);
       strcpy(tempDestDir, dest);
       strcat(path, "/");
                
       if (!strcmp(dentry->d_name, ".") || !strcmp(dentry->d_name, "..")){      
	 //want to ignore		 
          continue;
       }

	if(dentry->d_type == DT_DIR){ //do recursive call if dir
          strcat (tempSrcDir, dentry->d_name);
	  strcat (tempDestDir, dentry->d_name);
	  mkdir  (tempDestDir, S_IRWXG | S_IRWXO | S_IRWXU);
	  strcat (tempSrcDir, "/");
	  strcat (tempDestDir, "/");
	  cpDir  (tempSrcDir,tempDestDir);
	} 
        else {
          strcat (tempSrcDir, dentry->d_name);
	  strcat (tempDestDir, dentry->d_name);

          if (f1 = fopen(tempSrcDir, "r")){
             if(f2 = fopen(tempDestDir, "w+")){
                  char buf[BUFSIZE];
                   while(fgets(buf, sizeof(buf), f1) != 0)
			fputs(buf, f2);

		fclose(f1);
		fclose(f2);
	     }
	     else
	     {
		fprintf(stderr, "error: could not open destination file\n");
		fclose(f1);
	     }
          }
          else{
             fprintf(stderr, "warning: no read permission on %s\n", tempSrcDir);
          }
       }
     }

	closedir(dirPtr);
	return 0;
}

int main(int argc, char *argv[])
{
	int copyStatus;
        printf("%s\n", argv[0]);

        printf("%s\n", argv[1]);
        printf("%s\n", argv[2]);
        printf("%s\n", argv[3]);

	// Check arg count
	if (argc < 3 || argc > 4) {
		printf("Usage: ./mycp [-R | --help] source destination\n");
		exit(1);
	}

	if (argc == 4) {
		if (strcmp(argv[1], "--help") == 0) {
			printf("*** mycp --help ***\n");
			printf("Usage: ./mycp [-R | --help] source destination\n");
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[1], "-R") == 0) {
                   DIR *dSrc;
                   DIR *dDest;
		   char srcDir[512];
		   char destDir[512];
		   getcwd (srcDir, 511);
		   strcat (srcDir, "/");
		   strcpy (destDir, srcDir);
	  	   strcat (srcDir, argv[2]);
		   strcat (destDir, argv[3]);
		   strcat (srcDir, "/");
	       	   strcat (destDir, "/");
                   if(dSrc = opendir(argv[2])){
			if(dDest = opendir(argv[3])){
			   copyStatus = cpDir(srcDir, destDir);
			   if (copyStatus > 0)
				exit(1);
			   return 0;
			}

			if(ENOENT) //dest dir doesn't exist. need to create it.
			{
			   mkdir(argv[3], S_IRWXG | S_IRWXO | S_IRWXU);
			   dDest = opendir(argv[3]);
			   copyStatus = cpDir(srcDir, destDir);
			    if (copyStatus > 0)
				exit(1);
			   return 0;
			}

			if(EACCES) //don't have proper permission
			{
				fprintf(stderr, "Do not have proper permissions on source directory.\n");
				return 1;
			}
		}
		else //can't open src dir
		{
			fprintf(stderr, "Could not open source directory.\n");
			return 1;
		}
	     } 
             else {
			printf("Option not supported...\n");
			printf("Usage: ./mycp [-R | --help] source destination\n");
			printf("%s\n", argv[1]);
			exit(1);
		}
	} else {
		char* srcFile = (char*)malloc(strlen(argv[1]) + 1);
		char* destFile = (char*)malloc(strlen(argv[2]) + 1);
		strcpy(srcFile, argv[1]);
		strcpy(destFile, argv[2]);
		copyStatus = cpFile(srcFile, destFile);
		free(srcFile);
		free(destFile);
		if (copyStatus > 0)
			exit(1);
	}

	return 1;
}





