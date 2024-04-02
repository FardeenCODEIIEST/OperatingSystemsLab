#include <fcntl.h>   // for O_CREAT, O_WRONLY ,...
#include <unistd.h>  // for close(),read(),write(),...
#include <stdio.h>   // for printf(),scanf(),..
#include <string.h>  // for memset(),strcmp(),...
#include <stdlib.h>  // for exit(),...
#include <errno.h>   // for perror(),...
#include <stdbool.h> // for boolean value,...

#define MAX_FILES 128             // Maximum number of files we want in our filesystem
#define MAX_FILENAME_LENGTH 100   // Maximum length of file name
#define BLOCK_SIZE 1024           // Each block of 1KB
#define MAX_FILE_SIZE 1024 * 100  // Each file has max size of 100KB
#define FS_SIZE 1024 * 1024 * 100 // 100MB filesystem

// Structure of file in myfs filesystem
typedef struct
{
    char filename[MAX_FILENAME_LENGTH]; // name of the file
    int startBlock;                     // starting block number of the file in the filesystem
    int fileSize;                       // size of the file
    bool isOpen;                        // flag to determine whether file is open or not
    int currentCursor;                  // current cursor position in file
} myFileDesc;

// FileSystem Structure
typedef struct
{
    myFileDesc files[MAX_FILES]; // array of file descriptors
} myfs;

myfs filesystem; // filesystem
int myfsFd;      // file descriptor for the filesystem

/*
    @brief:-   Function to make the filesystem designated by the name 'disc_name'
    @details:- We create a dump OS file which will serve as a virtual disc for our filesystem
               We initialise the filesystem by marking its metadata in the superblock of the dump file
*/

void mymkfs(const char *disc_name)
{
    myfsFd = open(disc_name, O_RDWR | O_CREAT, 0666);
    if (myfsFd == -1)
    {
        perror("Failed to create the filesystem\n");
        exit(EXIT_FAILURE);
    }

    // Initialise the filesystem
    if (lseek(myfsFd, FS_SIZE - 1, SEEK_SET) == (off_t)-1)
    {
        perror("lseek error\n");
        exit(EXIT_FAILURE);
    }
    if (write(myfsFd, "\0", 1) == -1)
    {
        perror("write failed\n");
        exit(EXIT_FAILURE);
    }
    // Initialising the myfs struct and adding it to the beginning of the file
    memset(&filesystem, 0, sizeof(myfs));
    if (lseek(myfsFd, 0, SEEK_SET))
    {
        perror("lseek error\n");
        exit(EXIT_FAILURE);
    }
    if (write(myfsFd, &filesystem, sizeof(myfs)) == -1)
    {
        perror("write failed\n");
        exit(EXIT_FAILURE);
    }
    close(myfsFd);
}

/*
    @brief:-   Function to mount the filesystem for use
    @details:- By mounting we are opening the file with read, write permissions and initialising the filesystem descriptor with the meta data
*/

void mymount(const char *disc_name)
{
    myfsFd = open(disc_name, O_RDWR);
    if (myfsFd == -1)
    {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }

    // Load the myfs structure from the OS file
    if (read(myfsFd, &filesystem, sizeof(myfs)) == -1)
    {
        perror("read failed\n");
        exit(EXIT_FAILURE);
    }
}

/*
    @brief:-   Function to unmount the filesystem
    @details:- Unmounting refers to the fact of saving the updated meta-data of the filesystem and then closing its corresponding OS file descriptor
*/

void myunmount()
{
    if (lseek(myfsFd, 0, SEEK_SET) == (off_t)-1)
    {
        perror("lseek error\n");
        exit(EXIT_FAILURE);
    }
    if (write(myfsFd, &filesystem, sizeof(myfs)) == -1)
    {
        perror("write failed\n");
        exit(EXIT_FAILURE);
    }

    close(myfsFd);
}

/*
    @note:-    This is a flat file system, there is no concept of directories
    @brief:-   Function to list the files present in the filesystem
    @details:- We list the files using the file descriptor data corresponding to the filesystem
*/

void mylist(const char *disc_name)
{
    printf("The files present in the %s flat file system are:-\n", disc_name);
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (filesystem.files[i].filename[0] != '\0')
        {
            printf("File: %s, Size: %d Bytes\n", filesystem.files[i].filename, filesystem.files[i].fileSize);
        }
    }
}

/*
    @brief:-   Function to create file in the filesystem
    @details:- Searching for an empty slot or a file with the same name (to overwrite)
*/

int mycreat(const char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (strlen(filesystem.files[i].filename) == 0 || strcmp(filesystem.files[i].filename, name) == 0)
        {
            strncpy(filesystem.files[i].filename, name, MAX_FILENAME_LENGTH);
            filesystem.files[i].fileSize = 0;                   // New file has size 0
            filesystem.files[i].startBlock = i * MAX_FILE_SIZE; // Based on the fact that one file is allocated atmost 100 blocks
            filesystem.files[i].isOpen = false;                 // file is not opened yet
            filesystem.files[i].currentCursor = 0;              // current cursor is set at beginning of file
            return i;                                           // Return a file descriptor (index in this case)
        }
    }
    return -1; // No space
}

/*
    @brief:-   Function to return (if exists) filedescriptor for a file
    @details:- Linear searching the name of the file in the filsystem and make the cursor position 0 and open flag to be true
*/

int myopen(const char *filename)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (strcmp(filesystem.files[i].filename, filename) == 0)
        {
            filesystem.files[i].isOpen = true;     // open the file
            filesystem.files[i].currentCursor = 0; // start at the beginning
            return i;                              // File descriptor
        }
    }
    return -1; // File not found
}

/*
    @brief:-   Function to unlink(here delete) file in the filesystem
    @details:- Linear searching the name of the file in the filsystem and check if it is open
               if so then we nullify the metadata of the file in the filesystem super-block
*/

int myunlink(const char *filename)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (strcmp(filesystem.files[i].filename, filename) == 0)
        {
            if (filesystem.files[i].isOpen)
            {
                return -1; // Can't delete open files
            }
            memset(&filesystem.files[i], 0, sizeof(myFileDesc)); // "Delete" file by zeroing its metadata
            return 0;
        }
    }
    return -1; // File not found
}

/*
    @brief:-   Function to read the file pointed to by its file descriptor and yanking it into a buffer wirh "count" bytes length
    @details:- Check if the file is open and then checking whether the 'count' exceeds the file Size and if it exceeds we truncate the bytes read by some amount

*/

int myread(int fd, void *buf, int count)
{
    if (fd < 0 || fd >= MAX_FILES || !filesystem.files[fd].isOpen)
    {
        return -1; // File not open or invalid file descriptor
    }
    int readSize = count;
    // Adjust read size if it exceeds file size
    if (filesystem.files[fd].currentCursor + count > filesystem.files[fd].fileSize)
    {
        readSize = filesystem.files[fd].fileSize - filesystem.files[fd].currentCursor;
    }
    // Seek to the desired position
    if (lseek(myfsFd, filesystem.files[fd].startBlock + filesystem.files[fd].currentCursor, SEEK_SET) == -1)
    {
        return -1;
    }
    // read from the file and yank to buffer
    int result = read(myfsFd, buf, readSize);
    if (result != -1)
    {
        // cursor needs to be updated by the bytes read
        filesystem.files[fd].currentCursor += result;
    }
    // return the bytes read
    return result;
}

/*
    @brief:-   Function to write 'count' bytes of the buffer into the file pointed by itsfile descriptor
    @details:- Check if the file is open, seek to the desired position and then writing contents of buffer into the file

*/

int mywrite(int fd, const void *buf, int count)
{
    if (fd < 0 || fd >= MAX_FILES || !filesystem.files[fd].isOpen)
    {
        return -1; // File not open or invalid file descriptor
    }
    if (filesystem.files[fd].fileSize + count > MAX_FILE_SIZE)
    {
        return -1; // Cannot do the writing
    }
    // Seek to desired position
    if (lseek(myfsFd, filesystem.files[fd].startBlock + filesystem.files[fd].currentCursor, SEEK_SET) == -1)
    {
        return -1;
    }

    // Write to the buffer
    int result = write(myfsFd, buf, count);
    if (result != -1)
    {
        filesystem.files[fd].currentCursor += result; // Move cursor
        filesystem.files[fd].fileSize += result;      // Increase file size if needed
    }
    return result;
}

/*
    @brief:-   Function to set the cursor position in the file and returning the offset
    @details:- We adjust the cursor position of the file by offset bytes guided by whence parameter

*/

off_t mylseek(int fd, off_t offset, int whence)
{
    if (fd < 0 || fd >= MAX_FILES || !filesystem.files[fd].isOpen)
    {
        return -1; // File not open or invalid file descriptor
    }
    switch (whence)
    {
    case SEEK_SET:
        filesystem.files[fd].currentCursor = offset;
        break;
    case SEEK_CUR:
        filesystem.files[fd].currentCursor += offset;
        break;
    case SEEK_END:
        filesystem.files[fd].currentCursor = filesystem.files[fd].fileSize + offset;
        break;
    default:
        return -1;
    }
    // Edge cases
    if (filesystem.files[fd].currentCursor < 0)
    {
        filesystem.files[fd].currentCursor = 0;
    }
    if (filesystem.files[fd].currentCursor > filesystem.files[fd].fileSize)
    {
        filesystem.files[fd].currentCursor = filesystem.files[fd].fileSize;
    }
    // return the offset position
    return filesystem.files[fd].currentCursor;
}

/*
    @brief:-   Function to close the file
    @details:- We mark the file descriptor of the file's isOpen flag as false, to mark it closed

*/

int myclose(int fd)
{
    if (fd < 0 || fd >= MAX_FILES)
    {
        return -1; // Invalid file descriptor
    }
    filesystem.files[fd].isOpen = false; // mark it as closed
    return 0;
}

/*
    @brief:-   Function that copies contents of a file in myfs to another file in myfs
    @details:- Idea is to read from the source file and write to dest file in terms of blocks

*/

int mycopy(const char *source, const char *destination)
{
    // Open the corresponding file descriptors
    int srcFd = myopen(source);
    int destFd = mycreat(destination);

    if (srcFd == -1 || destFd == -1)
    {
        printf("Error opening source or destination file\n");
        return -1;
    }

    destFd = myopen(destination); // Open the newly created file for writing
    if (destFd == -1)
    {
        printf("Error opening newly created destination file\n");
        return -1;
    }

    char buffer[BLOCK_SIZE];
    int bytesRead;
    // Read from source file and write to dest file in terms of blocks
    while ((bytesRead = myread(srcFd, buffer, sizeof(buffer))) > 0)
    {
        mywrite(destFd, buffer, bytesRead);
    }
    // close the corresponding file descriptors
    myclose(srcFd);
    myclose(destFd);
    return 0;
}

/*
    @brief:-   Function that copies contents of a OS regular file to a file in myfs
    @details:- Idea is to read from the source file and write to dest file in terms of blocks

*/

int mycopyFromOS(const char *source, const char *destMyfs)
{
    // Open the corresponding file descriptors
    int srcFd = open(source, O_RDONLY);
    if (srcFd == -1)
    {
        perror("Error opening source file");
        return -1;
    }

    int destFd = mycreat(destMyfs);
    if (destFd == -1)
    {
        printf("Error creating destination file in myfs\n");
        close(srcFd);
        return -1;
    }
    destFd = myopen(destMyfs); // Open the newly created file for writing
    if (destFd == -1)
    {
        printf("Error opening newly created file in myfs\n");
        close(srcFd);
        return -1;
    }

    char buffer[BLOCK_SIZE];
    int bytesRead;
    // Read from source file and write to dest file in terms of blocks
    while ((bytesRead = read(srcFd, buffer, sizeof(buffer))) > 0)
    {
        mywrite(destFd, buffer, bytesRead);
    }
    // close the file descriptors
    close(srcFd);
    myclose(destFd);
    return 0;
}

/*
    @brief:-   Function that copies contents of a file in myfs to an OS regular file
    @details:- Idea is to read from the source file and write to dest file in terms of blocks

*/

int mycopyToOS(const char *sourceMyfs, const char *dest)
{

    // Open the corresponding file descriptors
    int srcFd = myopen(sourceMyfs);
    if (srcFd == -1)
    {
        printf("Error opening source file in myfs\n");
        return -1;
    }

    int destFd = open(dest, O_WRONLY | O_CREAT, 0666);
    if (destFd == -1)
    {
        perror("Error creating destination file");
        myclose(srcFd);
        return -1;
    }

    char buffer[BLOCK_SIZE];
    int bytesRead;

    // Read from source file and write to dest file in terms of blocks
    while ((bytesRead = myread(srcFd, buffer, sizeof(buffer))) > 0)
    {
        write(destFd, buffer, bytesRead);
    }

    // Close the file descriptors
    close(destFd);
    myclose(srcFd);
    return 0;
}

int main(int argc, char *argv[])
{
    // Usage:- <exe_name> <disc_name> <flag to say overwrite disc or restore it>
    if (argc != 3)
    {
        fprintf(stderr, "Usage:- %s <disc_name> <flag to say overwrite disc or restore it, 0 to overwrite disc metadata>\n", argv[0]);
        return 0;
    }
    // make the filesystem
    int fl = atoi(argv[2]);
    if (fl == 0)
        mymkfs(argv[1]);

    // mount the filesystem
    mymount(argv[1]);

    // Perform operations on the filesystem
    while (1)
    {
        printf("Enter:-\n 1. Copy a file in myfs to a file in myfs\n 2. Copy a regular OS file to a file in myfs\n 3. Copy a file in myfs to a regular OS file\n 4. Quit\n");
        int choice;
        scanf("%d", &choice);
        if (choice == 1)
        {
            printf("Copy from source to destination\n");
            printf("Enter the name of the source file\n");
            char srcFile[MAX_FILENAME_LENGTH];
            scanf("%s", srcFile);
            printf("Enter the name of the destination file\n");
            char destFile[MAX_FILENAME_LENGTH];
            scanf("%s", destFile);
            if (mycopy(srcFile, destFile) == -1)
            {
                fprintf(stderr, "mycopy failed\n");
                continue;
            }
            printf("File has been copied succesfully\n");
        }
        else if (choice == 2)
        {
            printf("Copy from source to destination\n");
            printf("Enter the name of the OS source file\n");
            char srcFile[MAX_FILENAME_LENGTH];
            scanf("%s", srcFile);
            printf("Enter the name of the destination file\n");
            char destFile[MAX_FILENAME_LENGTH];
            scanf("%s", destFile);
            if (mycopyFromOS(srcFile, destFile) == -1)
            {
                fprintf(stderr, "mycopyFromOS failed\n");
                continue;
            }
            printf("File from OS has been copied succesfully\n");
        }
        else if (choice == 3)
        {
            printf("Copy from source to destination\n");
            printf("Enter the name of the source file\n");
            char srcFile[MAX_FILENAME_LENGTH];
            scanf("%s", srcFile);
            printf("Enter the name of the OS regular destination file\n");
            char destFile[MAX_FILENAME_LENGTH];
            scanf("%s", destFile);
            if (mycopyToOS(srcFile, destFile) == -1)
            {
                fprintf(stderr, "mycopyToOS failed\n");
                continue;
            }
            printf("File from myfs to OS has been copied succesfully\n");
        }
        else if (choice == 4)
        {
            break;
        }
        else
        {
            printf("Try Again\n");
        }
        // list files every turn to see the updates
        mylist(argv[1]);
    }
    // unmount the filesystem
    printf("Unmounting the file system\n");
    myunmount();
    printf("GoodBye Thanks ....\n");

    return 0;
}