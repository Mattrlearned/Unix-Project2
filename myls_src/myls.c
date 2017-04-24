#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

int l_EN = 0;
int a_EN = 0;

void modeToLetters(int mode) {
	char str[20];
	strcpy(str, "----------");

	//permissions
	if (S_ISDIR(mode)) {
		str[0] = 'd';
	}
	if (S_ISCHR(mode)) {
		str[0] = 'c';
	}
	if (S_ISBLK(mode)) {
		str[0] = 'b';
	}
	if (mode & S_IRUSR) {
		str[1] = 'r';
	}
	if (mode & S_IWUSR) {
		str[2] = 'w';
	}
	if (mode & S_IXUSR) {
		str[3] = 'x';
	}
	if (mode & S_IRGRP) {
		str[4] = 'r';
	}
	if (mode & S_IWGRP) {
		str[5] = 'w';
	}
	if (mode & S_IXGRP) {
		str[6] = 'x';
	}
	if (mode & S_IROTH) {
		str[7] = 'r';
	}
	if (mode & S_IWOTH) {
		str[8] = 'w';
	}
	if (mode & S_IXOTH) {
		str[9] = 'x';
	}

	printf("%s ", str);
}

void displayError(const char *cDir) {
	if (errno & EACCES) {
		fprintf(stderr, "ls: cannot access %s: Permission denied.\n", cDir);
	}
	if (errno & ENOENT) {
		fprintf(stderr, "ls: cannot access %s: No such file or directory\n",
				cDir);
	}
}

int filter(const struct dirent *dir) {
	if (dir->d_name[0] == '.' && !a_EN) {
		return 0;
	}
	return 1;
}

int main(int argc, char **argv) {
	//ls
	char *cDir = ".";
	DIR *dirStr;
	struct stat buf;
	struct passwd *pwd;
	struct group *grp;
	struct dirent **nameList;
	char date[20];
	//parse options
	char c;
	opterr = 0;
	while ((c = (getopt(argc, argv, "la"))) != -1) {
		switch (c) {
		case 'l':
			l_EN = 1;
			break;
		case 'a':
			a_EN = 1;
			break;
		case '?':
			fprintf(stderr, "myls: invalid option -- \'%c\'\n", optopt);
			exit(-1);
			break;
		}
	}
	//get the pathname
	//nothing but flags
	if (argc > optind) {
		int size = strlen(argv[optind] + 1);
		cDir = malloc(size);
		strcpy(cDir, argv[optind]);
	}
	//ls -l
	//COUNT -> FILL -> SORT -> DISPLAY
	if (lstat(cDir, &buf) == -1) {
		displayError(cDir);
		exit(-1);
	}
	int mode = buf.st_mode;
	//it's a file
	if (S_ISREG(mode)) {
		pwd = getpwuid(buf.st_uid);
		grp = getgrgid(buf.st_gid);
		grp = getgrgid(buf.st_gid);
		strftime(date, 20, "%b %d %H:%M", localtime(&(buf.st_mtime)));
		if (l_EN) {
			modeToLetters(buf.st_mode);
			printf("%3d ", buf.st_nlink);
			printf("%-10s", pwd->pw_name);
			printf("%-10s", grp->gr_name);
			printf("%-10d", buf.st_size);
			printf("%s ", date);
			printf("%-20s\n", cDir);
		} else {
			printf("%s\n", cDir);
		}
		exit(0);
	}
	//else it's probably a dir
	chdir(cDir);

	int count = 0;
	if ((count = (scandir("./", &nameList, filter, alphasort))) < 0) {
		displayError(cDir);
		exit(-1);
	}
	int i = 0;

	for (i = 0; i < count; ++i) {
		lstat(nameList[i]->d_name, &buf);
		pwd = getpwuid(buf.st_uid);
		grp = getgrgid(buf.st_gid);
		strftime(date, 20, "%b %d %H:%M", localtime(&(buf.st_mtime)));
		if (l_EN) {
			modeToLetters(buf.st_mode);
			printf("%3d ", buf.st_nlink);
			printf("%-10s", pwd->pw_name);
			printf("%-10s", grp->gr_name);
			printf("%-10d", buf.st_size);
			printf("%s ", date);
			printf("%-20s\n", nameList[i]->d_name);
		} else {
			printf("%s\n", nameList[i]->d_name);
		}

	}
}

/*
 #include<stdio.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include<stdlib.h>
 #include <sys/types.h>
 #include <dirent.h>
 #include <sys/stat.h>

 int main(void) 
 { 

 DIR *dr = NULL;
 struct dirent *dptr = NULL;
 int i = 0;
 char *dir = NULL;

 struct winsize w;
 ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

 dir = getenv("PWD");
 if(NULL == dir)
 {
 printf("\n ");
 return 1;
 }

 dr = opendir((const char*)dir);
 if(NULL == dr)
 {
 printf("\n ERROR : Could not open the working directory\n");
 return -1;
 }

 for(i = 0; NULL != (dptr = readdir(dr)); i++)
 {
 if(dptr->d_name[0] != '.')
 {
 if(!access(dptr->d_name,X_OK))
 {
 int fd = -1;
 struct stat s;

 fd = open(dptr->d_name, O_RDONLY, 0);
 if(-1 == fd)
 {
 return 1;
 }

 fstat(fd, &s);
 if(S_ISDIR(s.st_mode))
 {
 printf("\e[1;34m""%s\t""\e[m",dptr->d_name);
 }
 else
 {
 printf("\e[1;32m""%s\t""\e[m",dptr->d_name);
 }
 close(fd);
 }
 else
 {
 printf("%s ",dptr->d_name);
 }
 }
 }
 printf("\n");

 return 0;
 }
 */
