#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void myls(int opts, int argNum, char **fileName){
     struct dirent **namelist;
        int n;
        if((n = scandir(fileName[argNum], &namelist, 0, alphasort)) < 0 ){ //is not a directory
            if( access(fileName[argNum], F_OK) != -1){ //do we have permission to read the file?
                if((opts & 1) != 0){ //user passed option "-l", printing long form      
                    struct stat statbuf;
                    stat(fileName[argNum], &statbuf);
                    char modifiedDate[20];
                    strftime(modifiedDate, 20, "%b %d %H:%M", localtime(&(statbuf.st_mtime)));

                    printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                    printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                    printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                    printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                    printf("%3d ", (int)statbuf.st_nlink);
                    struct passwd pwd = *getpwuid(statbuf.st_uid);
                    printf(" %10s", pwd.pw_name);
                    //printf("%5s ", getpwuid(statbuf.st_uid)->pw_name);
                    printf("%10s ", getgrgid(statbuf.st_gid)->gr_name);
                    printf("%6lld ", (long long)statbuf.st_size);
                    printf("%10s ", modifiedDate);
                    printf("%s\n", fileName[argNum]);
                }
                else { //no option passed. Print short form ls
                    printf("%s  ", fileName[argNum]);
                    if(argNum == 0)
                        printf("\n");
                }
               
            }
            else{ //file doesn't exist
                fprintf(stderr, "myls: %s: No such file or directory\n", fileName[argNum]);
                exit(0);
            }
        }
        else{ //file is a directory
            int i;
            for( i = 0; i < n; i++){
                 if(((opts & 2) != 0)  || ((namelist[i]->d_name)[0] != '.' )){   
                 if((opts & 1) != 0){
                        struct stat statbuf;
                        stat(namelist[i]->d_name, &statbuf);
                        char modifiedDate[20];
                        strftime(modifiedDate, 20, "%b %d %H:%M", localtime(&(statbuf.st_mtime)));

                        printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                        printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                        printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                        printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                        printf("%2ld ", (long)statbuf.st_nlink);
                        printf("%5s ", getpwuid(statbuf.st_uid)->pw_name);
                        printf("%6s ", getgrgid(statbuf.st_gid)->gr_name);
                        printf("%6lld ", (long long)statbuf.st_size);
                        printf("%10s ", modifiedDate);
                        printf("%s\n", namelist[i]->d_name);
                    }
                    else {
                        printf("%s  ", namelist[i]->d_name);
                        if( i == (n-1))
                            printf ("\n");
                    }
                
                    free(namelist[i]);
                }
            }

            free(namelist);
        }
        //recursive call to next file in character array
        if(argNum > 0)
            myls(opts, --argNum, fileName);

}

int main(int argc, char *argv[]){
    
    int c;
    extern int optind;
    extern char *optarg;

    int lOpt = 0;
    int aOpt =0;
    int opts = 0;
   
    while((c = getopt(argc, argv, "la")) > 0){
      
        switch(c){

            case 'l':
                lOpt = 1;
                opts |= lOpt;
                break;
            case 'a':
                aOpt = 2;
                opts |= aOpt;
                break;
            case '?':
                exit(1);
                break;
        }
    }

    //no file given
    if( (argc - optind -1) < 0){
        char *file[] = {"."}; //default current directory
        myls(opts, 0, file);
    }
    else //at least one file given
        myls(opts, (argc - optind - 1), &argv[optind]);

    exit(0);
}
