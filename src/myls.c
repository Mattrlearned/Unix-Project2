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

struct fTuple
{
	char permissions[10];
	int links;
	char *uname;
	char *gname;
	int size;
	char date[20];
	char *fName;
};


void modeToLetters(int mode)
{
	char str[10];
	strcpy(str, "----------");

	//permissions
	if(S_ISDIR(mode)){ str[0] = 'd'; }
	if(S_ISCHR(mode)){ str[0] = 'c'; }
	if(S_ISBLK(mode)){ str[0] = 'b'; }
	if(mode & S_IRUSR){ str[1] = 'r'; }
	if(mode & S_IWUSR){ str[2] = 'w'; }
	if(mode & S_IXUSR){ str[3] = 'x'; }
	if(mode & S_IRGRP){ str[4] = 'r'; }
	if(mode & S_IWGRP){ str[5] = 'w'; }
	if(mode & S_IXGRP){ str[6] = 'x'; }
	if(mode & S_IROTH){ str[7] = 'r'; }
	if(mode & S_IWOTH){ str[8] = 'w'; }
	if(mode & S_IXOTH){ str[9] = 'x'; }

	printf("%s", str);

}

void printTuple(struct fTuple *tuple)
{
	printf("%10s %-3d %-10s %-10s %-10d %s%-20s\n", tuple->permissions, tuple->links, tuple->uname, tuple->gname, tuple->size, tuple->date, tuple->fName);
}
void print(struct fTuple *arr, int count)
{
	if(l_EN)
	{
		printf("total %d\n", count);
	}
	int i = 0;
	for(i = 0; i < count; ++i)
	{
		if(l_EN)
		{
			printTuple(&arr[i]);
		}
		else
		{
			printf("%s\n", arr[i].fName);
		}
	}

	for(i = 0; i < count; ++i)
	{
		free(arr[i].uname);
		free(arr[i].gname);
	}
	free(arr);
}
int compar(const void* p1, const void* p2)
{
	struct fTuple f1 = *(struct fTuple*)p1;
	struct fTuple f2 = *(struct fTuple*)p2;
	int r = strcmp(f1.fName, f2.fName);

	return r;
}

void displayError(const char *cDir)
{
	if(errno & EACCES)
	{
		fprintf(stderr, "myls: cannot access %s: Permission denied.\n", cDir);
	}
	if(errno & ENOENT)
	{
		fprintf(stderr, "myls: cannot access %s: No such file or directory\n", cDir);
	}
}        

int main(int argc, char **argv)
{
	//ls
	char *cDir = ".";
	DIR *dirStr;
	struct fTuple file;
	struct stat buf;
	struct passwd *pwd;
	struct group *grp;
	struct dirent *dirEnt;

	//parse options
	char c;
	opterr = 0;
	while((c = (getopt(argc, argv, "la"))) != -1)
	{
		switch(c)
		{
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
	if(argc > optind)
	{
		int size = strlen(argv[optind] + 1);
		cDir = malloc(size);
		strcpy(cDir, argv[optind]);
	}
	//ls -l
	//COUNT -> FILL -> SORT -> DISPLAY
	lstat(cDir, &buf);
	int mode = buf.st_mode;
	//it's a file
	//else it's probably a dir
	char *oldDir = getcwd(NULL, 0);

	struct dirent **nameList;
	chdir(cDir);

	int count = 0;
	if ((dirStr = opendir(cDir)) != 0)
	{
		//get directory size
		if((count = (scandir(cDir, &nameList, NULL, alphasort))) < 0)
		{
			fprintf(stderr, "can't open %s", cDir);
			exit(-1);
		}
	}
	else
	{
		displayError(cDir);
		exit(-1);
	}
	//FILL ARRAY
	int i = 0;
	char date[20];
	for(i = 0; i < count; ++i)
	{
		lstat(dirEnt->d_name, &buf);
		//fill file tuple
		modeToLetters(buf.st_mode);
		printf("%5d", buf.st_nlink);
		pwd = getpwuid(buf.st_uid);
		grp = getgrgid(buf.st_gid);

		printf("%20s", pwd->pw_name);
		printf("%20s", grp->gr_name);

		strftime(date, 20, "%b %d %H:%M", localtime(&(buf.st_mtime)));
		printf("%s\n", nameList[i]->d_name);
	}
}



