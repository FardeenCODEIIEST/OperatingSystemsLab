#include <sys/types.h> // for types ,..
#include <sys/stat.h>  // for stat buffer, lstat ,..
#include <stdio.h>     // for printf(),..
#include <errno.h>     // for errno,..
#include <stdlib.h>    // for exit ,..
#include <dirent.h>    // for dir* dirent* ,..
#include <string.h>    // for strcmp(),..
#include <stdbool.h>   // for bool
#include <fnmatch.h>   // for fnmatch , pattern matching ,..
#include <time.h>      // for time_t

#define MAX_BUFFER_SIZE 1000 // max buffer size

void recursivelySearch(char *dirPath, bool flag, const char *pattern, time_t init, mode_t perm)
{
    DIR *dir;
    struct dirent *directEntry;
    struct stat inode;
    char buffer[MAX_BUFFER_SIZE];
    // printf("%s\n", pattern);
    dir = opendir(dirPath);
    if (dir == NULL)
    {
        perror("opendir() error:\n");
        exit(0);
    }

    while ((directEntry = readdir(dir)) != 0)
    {
        sprintf(buffer, "%s/%s", dirPath, directEntry->d_name);
        lstat(buffer, &inode);

        // test each entry

        if (S_ISDIR(inode.st_mode) && strcmp(directEntry->d_name, ".") != 0 && strcmp(directEntry->d_name, "..") != 0)
        {
            // directory
            if ((pattern == NULL || fnmatch(pattern, directEntry->d_name, 0) == 0) && (init == -1 || difftime(time(NULL), inode.st_mtime) > init))
                printf("%s/%s\n", dirPath, directEntry->d_name);
            if (flag)
                recursivelySearch(buffer, flag, pattern, init, perm);
        }
        else if ((pattern == NULL || fnmatch(pattern, directEntry->d_name, 0) == 0) && S_ISREG(inode.st_mode) && ((init == -1 || difftime(time(NULL), inode.st_mtime) > init)) && (inode.st_mode && 0777 == perm))
        {
            // file
            // printf("%s matches %s\n", directEntry->d_name, pattern);
            printf("%s/%s\n", dirPath, directEntry->d_name);
        }
    }
    closedir(dir);
    return;
}

int main(int argc, char **argv)
{
    // Usage : exe_name [path] [options]
    // -nr             ---> not-recursive (by default it is recursive)
    // -name pattern   ---> pattern - matching
    // -mtime m        ---> search for files before,after and exactly at modification time m*24 hours
    // -perm p         ---> search for files, directories whose permission matches p

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <path of the directoy> <-nr> <-name [pattern]> <-mtime [-/+]days > <-perm [p]>\n", argv[0]);
        return 0;
    }
    bool recursive = true;
    const char *pattern = NULL;
    time_t init = -1;
    mode_t perm = 0;
    printf("The directory contents are :\n");
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-nr") == 0)
        {
            recursive = false;
        }
        else if (strcmp(argv[i], "-name") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "Please include the pattern\n");
                return 0;
            }
            pattern = argv[i + 1];
        }
        else if (strcmp("-mtime", argv[i]) == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "Please give time\n");
                return 0;
            }
            char sign;
            sscanf(argv[i + 1], "%c%ld", &sign, &init);
            //  if (sscanf(argv[i + 1], "%c%ld", &sign, &init) != 2)
            // {
            //     fprintf(stderr, "Wrong Input\n");
            //     return 0;
            // }
            init = (sign == '-') ? -init : init;
        }
        else if (strcmp("-perm", argv[i]) == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "Please give permissions\n");
                return 0;
            }
            sscanf(argv[i + 1], "%o", &perm);
            printf("%ho\n", perm);
        }
    }
    recursivelySearch(argv[1], recursive, pattern, init, perm);
    return 0;
}
