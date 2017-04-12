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

struct fTuple
{
    char permissions[10];
    int links;
    char *uname;
    char *gname;
    int size;
    char *mTime;
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
    char *tempStr;
    tempStr = malloc(sizeof(tuple->mTime));
    strcpy(tempStr, tuple->mTime);
    char *newLine = strchr(tempStr, '\n');
    *newLine = ' ';
    printf("%10s %-3d %-10s %-10s %-10d %s%-20s\n", tuple->permissions, tuple->links, tuple->uname, tuple->gname, tuple->size, tempStr, tuple->fName); 
    free(tempStr);
}

int compar(const void* p1, const void* p2)
{
    struct fTuple f1 = *(struct fTuple*)p1;
    struct fTuple f2 = *(struct fTuple*)p2;
    int r = strcmp(f1.fName, f2.fName);

    return r;
}

int main(int argc, char **argv)
{
    //COUNT -> FILL -> SORT -> DISPLAY
    char *cDir = "."; 
    DIR *dirStr;
    struct fTuple file; 
    struct stat buf;
    struct passwd *pwd;
    struct group *grp;
    struct dirent *dirEnt;
    int count = 0;
    if ((dirStr = opendir(cDir)) != 0)
    {
        //get directory size
        while((dirEnt = readdir(dirStr)) != 0)
        {
            if(dirEnt->d_name[0] == '.')
            {
            }
            else
            {
                count++;
            }
        }
        rewinddir(dirStr);
    }
    //FILL ARRAY
    struct fTuple *fArr;
    fArr = malloc(count * sizeof(struct fTuple));
    int i = 0;
    for(i = 0; i < count; ++i)
    {
        dirEnt = readdir(dirStr);
        if(dirEnt->d_name[0] == '.')
        {
            //ignore entry and keep array index same
            i--;
        }
        else
        {
            stat(dirEnt->d_name, &buf);
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
            int mSize = strlen(ctime(&(buf.st_mtim)) + 1); //get ctime's size
            fArr[i].mTime = malloc(mSize + 1);
            strcpy(fArr[i].mTime, ctime(&(buf.st_mtim)));
            fArr[i].fName = dirEnt->d_name;
        }
    }
    //use stdlib qsort because why not
    qsort((void *)fArr, count, sizeof(struct fTuple), compar);

    printf("total %d\n", count);
    for(i = 0; i < count; ++i)
    {
        printTuple(&fArr[i]);
    }

    for(i = 0; i < count; ++i)
    {
        free(fArr[i].uname);
        free(fArr[i].gname);
        free(fArr[i].mTime);
    }
    free(fArr);
    
}

