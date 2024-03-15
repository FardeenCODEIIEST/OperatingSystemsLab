#include <stdio.h>     // for printf(),scanf(),...
#include <stdlib.h>    // for exit(), ...
#include <fcntl.h>     // for open(),...
#include <unistd.h>    // for read(),write(),lseek(),..
#include <sys/types.h> // for types ...
#include <string.h>    // for memset(),..
#include <time.h>      // for time(),..

int fileFD; // file descriptor

/*
    @brief:- Function initialises the file <fname> with its meta data bsize anf bno in the first 1024 bytes
    @param:- fileName: const char*, blockSize: int, Number of blocks:int
    @return:- On success return 0, else return -1

*/
int init_File_dd(const char *fname, int bsize, int bno)
{
    fileFD = open(fname, O_RDWR | O_CREAT, 0666);
    if (fileFD == -1)
    {
        perror("Error opening file\n");
        return -1;
    }
    // Write the block size and block number in the file
    if (write(fileFD, &bsize, sizeof(int)) == -1)
    {
        perror("Error writing block size \n");
        return -1;
    }
    if (write(fileFD, &bno, sizeof(int)) == -1)
    {
        perror("Error writing number of blocks\n");
        return -1;
    }

    // Pad the file to set its size
    char *buffer1 = (char *)malloc(sizeof(char) * 1016);
    memset(buffer1, 0, sizeof(char) * 1016);

    if (write(fileFD, buffer1, sizeof(char) * 1016) == -1)
    {
        perror("Error padding file\n");
        return -1;
    }
    char *buffer2 = (char *)malloc(sizeof(char) * bsize);
    memset(buffer2, 0, sizeof(char) * bsize);
    for (int i = 0; i < bno; i++)
    {
        if (write(fileFD, buffer2, sizeof(char) * bsize) == -1)
        {
            perror("Error padding file\n");
            return -1;
        }
    }
    free(buffer1);
    free(buffer2);
    close(fileFD);
    return 0;
}

/*
    @brief:- Function reads the <fname> file and reads the bno th block (0 indexed) from file and puts it into the buffer
    @param:- fileName: const char*, blockIndex: int, readBuffer: char*
    @return:- On success return 1, else return 0
*/

int read_block(const char *fname, int bno, char *buffer)
{
    fileFD = open(fname, O_RDONLY, 0666);
    if (fileFD == -1)
    {
        perror("Error opening file\n");
        return 0;
    }
    int blockSize, blocks;
    // read block size and block number
    if (read(fileFD, &blockSize, sizeof(int)) == -1)
    {
        perror("Error reading block size\n");
        return 0;
    }
    printf("The block size is:%d\n", blockSize);
    if (read(fileFD, &blocks, sizeof(int)) == -1)
    {
        perror("Error reading number of blocks\n");
        return 0;
    }
    printf("The number of blocks is:%d\n", blocks);
    // edge case checking
    if (bno >= blocks)
    {
        fprintf(stderr, "Block index out of bound\n");
        return 0;
    }
    off_t offset;
    // seek to the correct position
    offset = lseek(fileFD, 1016 + (bno - 1) * blockSize, SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    if (read(fileFD, buffer, blockSize) == -1)
    {
        perror("Error reading buffer\n");
        return 0;
    }
    close(fileFD);
    return 1;
}

/*
    @brief:- Function writes to the bno th block (0 indexed) in the <fname> file taking values from the buffer
    @param:- fileName: const char*, blockIndex: int, writeBuffer: char*
    @return:- On success return 1, else return 0
*/
int write_block(const char *fname, int bno, char *buffer)
{
    fileFD = open(fname, O_RDWR, 0666);
    if (fileFD == -1)
    {
        perror("Error opening file\n");
        return 0;
    }
    int blockSize, blocks;
    // read block size and block number
    if (read(fileFD, &blockSize, sizeof(int)) == -1)
    {
        perror("Error reading block size\n");
        return 0;
    }
    if (read(fileFD, &blocks, sizeof(int)) == -1)
    {
        perror("Error reading number of blocks\n");
        return 0;
    }
    printf("The number of blocks is:%d\n", blocks);
    // edge case checking
    if (bno >= blocks)
    {
        fprintf(stderr, "Block index out of bound\n");
        return 0;
    }
    off_t offset;
    // seek to the block position
    offset = lseek(fileFD, 1016 + (bno - 1) * blockSize, SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    if (write(fileFD, buffer, blockSize) == -1)
    {
        perror("Error writing block size\n");
        return 0;
    }
    close(fileFD);
    return 1;
}

int main(int argc, char *argv[])
{
    // Usage:- [exe_name] [file_name]
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <FileName>\n", argv[0]);
        return 0;
    }

    srand(time(NULL));
    printf("Enter the file information\n");
    int bsize, bno;
    printf("Enter the block size\n");
    scanf("%d", &bsize);
    printf("Enter the number of blocks\n");
    scanf("%d", &bno);

    if (init_File_dd(argv[1], bsize, bno) == -1)
    {
        fprintf(stderr, "Error creating file\n");
        exit(EXIT_FAILURE);
    }
    printf("File creation successful\n");
    while (1)
    {
        int choice;
        printf("Enter the following choices:-\n 1. Read Block\n 2. Write Block\n 3. Exit\n");
        scanf("%d", &choice);
        if (choice == 1)
        {
            int bnoIn;
            printf("Enter the block number\n");
            scanf("%d", &bnoIn);
            char *buffer = (char *)malloc(bsize * sizeof(char));
            if (read_block(argv[1], bnoIn, buffer) == 0)
            {
                fprintf(stderr, "Reading block from file is unsuccessful\n");
                exit(EXIT_FAILURE);
            }
            printf("The buffer read is :\n %s\n", buffer);
            free(buffer);
        }
        else if (choice == 2)
        {
            int bnoIn;
            printf("Enter the block number\n");
            scanf("%d", &bnoIn);
            char *buffer = (char *)malloc(sizeof(char) * bsize);
            int ch = rand() % 26;
            char pad = ('a' + ch);
            memset(buffer, pad, bsize);
            if (write_block(argv[1], bnoIn, buffer) == 0)
            {
                fprintf(stderr, "Writing block to file is unsuccessful\n");
                exit(EXIT_FAILURE);
            }
            printf("Block has been written successfully\n");
            free(buffer);
        }
        else if (choice == 3)
        {
            printf("Goodbye ...\n");
            break;
        }
        else
        {
            printf("Try Again\n");
        }
    }
}
