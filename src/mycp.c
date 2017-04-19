#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 1024
#define COPYBUF 0644

int getstat(char *filename, char* fileN)
{
    struct stat fileInfo;
    if ( (strcmp(fileN, ".") == 0) || (strcmp(fileN, "..") == 0) )
    	return -1;

    //printf("Next File %s\n", filename);
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

// Helper
char* stringConcat(const char* s1, const char* s2) {
	char* final = (char *)malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(final, s1);
	strcat(final, s2);
	return final;
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
	int fileStatus;
	char* tempDestDir = (char*)malloc(strlen(dest) + 1);
	char* destFilePath = (char*)malloc(strlen(dest) + 1);
	char* tempSrcDir = (char*)malloc(strlen(src) + 1);
	char* srcFilePath = (char*)malloc(strlen(dest) + 1);
	char* path = (char*)malloc(strlen(src) + 1);
	strcpy(tempDestDir, dest);
	strcpy(tempSrcDir, src);
	strcpy(path, src);
	char* endPtr;

	tempDestDir = stringConcat(tempDestDir, "/");
	tempSrcDir = stringConcat(tempSrcDir, "/");
	path = stringConcat(path, "/");

	// Check stat to verify src is a directory
	// If not, then handle as an error
	struct stat dirInfo;
	if (lstat(src, &dirInfo) < 0) {
		perror("Cannot open directory");
	}

	if (!S_ISDIR(dirInfo.st_mode)) {
		printf("%s is not a directory.\n", src);
		printf("Usage: ./mycp [-R | --help] source destination\n");
		return 1;
	}

	// Check stat to verify dest is created or not
	if (lstat(dest, &dirInfo) < 0) {
		mkdir(dest, 0700);
	}

	char* fileN;
	// printf("Right before copying files in cpDir()\n");
 	// printf("Before strcat tempDestDir=%s\n", tempDestDir);

	dirPtr = opendir(src);
	if (dirPtr == NULL) {
		fprintf(stderr, "Error opening source directory %s\n", src);
		perror(src);
	}
	else {
		while( (dentry = readdir(dirPtr)) ) {

			fileN = dentry->d_name;

			fileStatus = getstat(path, fileN);

			if ( fileStatus == 0) {
				destFilePath = stringConcat(tempDestDir, dentry->d_name);
				printf("after strcat tempDestDir=%s\n", tempDestDir); 
				srcFilePath = stringConcat(tempSrcDir, dentry->d_name);
				printf("srcFilePath: %s\n", srcFilePath);
				cpFile(srcFilePath, destFilePath);
			} else if ( fileStatus == 1 ) {
				tempDestDir = stringConcat(tempDestDir, "/");
				tempDestDir = stringConcat(tempDestDir, dentry->d_name);
				tempSrcDir = stringConcat(tempSrcDir, "/");
				tempSrcDir = stringConcat(tempSrcDir, dentry->d_name);
				if (lstat(tempDestDir, &dirInfo) < 0) {
					mkdir(tempDestDir, 0700);
					printf("Created directory: %s\n", tempDestDir);
				}
			}
		}
	}

	// Close directory & free dynamically allocated memory
	free(tempSrcDir);
	free(tempDestDir);
	free(path);
	closedir(dirPtr);
	return 0;
}

int main(int argc, char const *argv[])
{
	int copyStatus;

	// Check arg count
	if (argc < 3 || argc > 4) {
		printf("Usage: ./mycp [-R | --help] source destination\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 4) {
		if (strcmp(argv[1], "--help") == 0) {
			printf("*** mycp --help ***\n");
			printf("Usage: ./mycp [-R | --help] source destination\n");
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[1], "-R") == 0) {
			char* srcDir = (char*)malloc(strlen(argv[2]) + 1);
			char* destDir = (char*)malloc(strlen(argv[3]) + 1);
			strcpy(srcDir, argv[2]);
			strcpy(destDir, argv[3]);
			copyStatus = cpDir(srcDir, destDir);
			free(srcDir);
			free(destDir);
			if (copyStatus > 0)
				exit(EXIT_FAILURE);
		} else {
			printf("Option not supported...\n");
			printf("Usage: ./mycp [-R | --help] source destination\n");
			printf("%s\n", argv[1]);
			exit(EXIT_FAILURE);
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
			exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}





