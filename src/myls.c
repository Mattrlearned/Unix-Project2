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


void modeToLetters(int mode, char *str)
{
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

}

void printTuple(struct fTuple *tuple)
{
    printf("%10s %-3d %-10s %-10s %-10d %s %-20s\n", tuple->permissions, tuple->links, tuple->uname, tuple->gname, tuple->size, tuple->date, tuple->fName); 
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
        fprintf(stderr, "ls: cannot access %s: Permission denied.\n", cDir);
    }
    if(errno & ENOENT)
    {
        fprintf(stderr, "ls: cannot access %s: No such file or directory\n", cDir);
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
    if(S_ISREG(mode))
    {
        struct fTuple file;
        modeToLetters(buf.st_mode, file.permissions);
        file.links = buf.st_nlink;
        pwd = getpwuid(buf.st_uid);
        grp = getgrgid(buf.st_gid);

        int uSize = strlen(pwd->pw_name);
        file.uname = malloc(uSize + 1);//store size + null
        strcpy(file.uname, pwd->pw_name);//copy pw_name into array

        int gSize = strlen(grp->gr_name);
        file.gname = malloc(gSize + 1);
        strcpy(file.gname, grp->gr_name);
        file.size = buf.st_size;
        strftime(file.date, 20, "%b %d %H:%M", localtime(&(buf.st_mtime)));
        file.fName = cDir;
        if(l_EN)
        {
            printTuple(&file);
        }
        else
        {
            printf("%s\n", cDir);
        }
        exit(0);
    } 
    //else it's probably a dir
    char *oldDir = getcwd(NULL, 0);
   
    chdir(cDir); 

    struct dirent **nameList;
    int count = 0;
    if((count = (scandir(cDir, &nameList, NULL, alphasort))) < 0)
    {
        displayError(cDir);
        exit(-1);
    }
    //FILL ARRAY
    struct fTuple *fArr;
    fArr = malloc(count * sizeof(struct fTuple));
    int i = 0;
    for(i = 0; i < count; ++i)
    {
            lstat(nameList[i]->d_name, &buf);
            //fill file tuple
            modeToLetters(buf.st_mode, fArr[i].permissions);
            fArr[i].links = buf.st_nlink;
            pwd = getpwuid(buf.st_uid);
            grp = getgrgid(buf.st_gid);

            int uSize = strlen(pwd->pw_name);
            fArr[i].uname = malloc(uSize + 1);//store size + null
            strcpy(fArr[i].uname, pwd->pw_name);//copy pw_name into array

            int gSize = strlen(grp->gr_name);
            fArr[i].gname = malloc(gSize + 1);
            strcpy(fArr[i].gname, grp->gr_name);
            fArr[i].size = buf.st_size;

            strftime(fArr[i].date, 20, "%b %d %H:%M", localtime(&(buf.st_mtime)));

            fArr[i].fName = nameList[i]->d_name;
    }
    //use stdlib qsort because why not

    print(fArr, count);
}

