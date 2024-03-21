#include <stdio.h>     // for printf(),scanf(),...
#include <stdlib.h>    // for exit(), ...
#include <fcntl.h>     // for open(),...
#include <unistd.h>    // for read(),write(),lseek(),..
#include <sys/types.h> // for types ...
#include <string.h>    // for memset(),..
#include <time.h>      // for time(),..
#include <stdint.h>    // for standard interger types....
#include <stdbool.h>   //

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
    if (write(fileFD, &bno, sizeof(int)) == -1)
    {
        perror("Error writing number of blocks\n");
        return -1;
    }
    if (write(fileFD, &bsize, sizeof(int)) == -1)
    {
        perror("Error writing block size \n");
        return -1;
    }
    // Write the number of free blocks
    if (write(fileFD, &bno, sizeof(int)) == -1)
    {
        perror("Error writing number of free blocks\n");
        return -1;
    }
    // Writing the bit pattern
    uint8_t pattern[bno / 8];
    for (int i = 0; i < bno / 8; i++)
    {
        pattern[i] = 0; // making all of them free
    }
    if (write(fileFD, pattern, sizeof(uint8_t) * (bno / 8)) == -1)
    {
        perror("Error writing the bit pattern\n");
        return -1;
    }
    // Writing the block numbers of the chain
    for (int i = 0; i < 256 - (bno / 32) - 3; i++)
    {
        int chain_index = -1;
        if (write(fileFD, &chain_index, sizeof(int)) == -1)
        {
            perror("write failed\n");
            return -1;
        }
    }
    close(fileFD);
    return 0;
}

/*
    @brief:- Function reads the <fname> file and reads the bno th block (0 indexed) from file and puts it into the buffer
    @param:- fileName: const char*, blockIndex: int, readBuffer: char*,chaintIndex:int
    @return:- On success return 1, else return 0
*/

int read_block(const char *fname, int chno, int bno, char *buffer)
{
    fileFD = open(fname, O_RDONLY, 0666);
    if (fileFD == -1)
    {
        perror("Error opening file\n");
        return 0;
    }
    int blockSize, blocks, freeBlocks;
    // read block size and block number
    if (read(fileFD, &blocks, sizeof(int)) == -1)
    {
        perror("Error reading number of blocks\n");
        return 0;
    }
    printf("The number of blocks is:%d\n", blocks);
    if (read(fileFD, &blockSize, sizeof(int)) == -1)
    {
        perror("Error reading block size\n");
        return 0;
    }
    printf("The block size is:%d\n", blockSize);
    if (read(fileFD, &freeBlocks, sizeof(int)) == -1)
    {
        perror("Error reading number of free blocks\n");
        return 0;
    }
    printf("The number of free blocks is:%d\n", freeBlocks);
    uint8_t pattern[blocks / 8];
    if (read(fileFD, pattern, sizeof(uint8_t) * (blocks / 8)) == -1)
    {
        perror("Error reading free block pattern\n");
        return 0;
    }
    // edge case checking
    if (bno >= blocks)
    {
        fprintf(stderr, "Block index out of bound\n");
        return 0;
    }
    int chain_block_no;
    for (int i = 0; i <= chno; i++)
    {
        if (read(fileFD, &chain_block_no, sizeof(int)) == -1)
        {
            perror("Error reading the block number\n");
            return 0;
        }
    }
    bool f = false;
    off_t offset, offset_Rem;
    offset = lseek(fileFD, 12 + (bno / 8) + chno * sizeof(int), SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    printf(" Start Block number is %d in chain %d\n", chain_block_no, chno);
    while (chain_block_no != -1)
    {
        if (chain_block_no == bno)
        {
            offset_Rem = offset;
            f = true;
            break;
        }
        offset = lseek(fileFD, 1020 + (chain_block_no + 1) * blockSize, SEEK_SET);
        if (offset == (off_t)-1)
        {
            perror("lseek error\n");
            return 0;
        }
        if (read(fileFD, &chain_block_no, sizeof(int)) == -1)
        {
            perror("read error\n");
            return 0;
        }
        printf("Block number %d in chain %d\n", chain_block_no, chno);
    }
    if (!f)
    {
        fprintf(stderr, "Block not found\n");
        return 0;
    }
    offset = lseek(fileFD, offset_Rem + 4, SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    if (read(fileFD, buffer, blockSize - 4) == -1)
    {
        perror("Error reading buffer\n");
        return 0;
    }
    close(fileFD);
    return 1;
}

/*
    @brief:- Function writes to the bno th block (0 indexed) in the <fname> file taking values from the buffer
    @param:- fileName: const char*, blockIndex: int, writeBuffer: char*, chainIndex:int
    @return:- On success return 1, else return 0
*/
int write_block(const char *fname, int chno, int bno, char *buffer)
{
    fileFD = open(fname, O_RDWR, 0666);
    if (fileFD == -1)
    {
        perror("Error opening file\n");
        return 0;
    }
    int blockSize, blocks, freeBlocks;
    // read block size and block number
    if (read(fileFD, &blocks, sizeof(int)) == -1)
    {
        perror("Error reading number of blocks\n");
        return 0;
    }
    printf("The number of blocks is:%d\n", blocks);
    if (read(fileFD, &blockSize, sizeof(int)) == -1)
    {
        perror("Error reading block size\n");
        return 0;
    }
    printf("The block size is:%d\n", blockSize);
    if (read(fileFD, &freeBlocks, sizeof(int)) == -1)
    {
        perror("Error reading number of free blocks\n");
        return 0;
    }
    printf("The number of free blocks is:%d\n", freeBlocks);
    uint8_t pattern[blocks / 8];
    if (read(fileFD, pattern, sizeof(uint8_t) * (blocks / 8)) == -1)
    {
        perror("Error reading free block pattern\n");
        return 0;
    }
    // edge case checking
    if (bno >= blocks)
    {
        fprintf(stderr, "Block index out of bound\n");
        return 0;
    }
    // free block check
    int p_byte_check = bno / 8;
    int p_byte_pos = bno % 8;
    if ((pattern[p_byte_check] >> p_byte_pos) & 0x01 == 1)
    {
        fprintf(stderr, "block is busy cannot write\n");
        return 0;
    }

    int chain_block_no;
    off_t offset;
    for (int i = 0; i <= chno; i++)
    {
        if (read(fileFD, &chain_block_no, sizeof(int)) == -1)
        {
            perror("Error creaing the block number\n");
            return 0;
        }
    }

    printf("Start block number is %d in chain %d\n", chain_block_no, chno);
    while (chain_block_no != -1)
    {
        offset = lseek(fileFD, 1020 + (chain_block_no + 1) * blockSize, SEEK_SET);
        if (offset == (off_t)-1)
        {
            perror("lseek error\n");
            return 0;
        }
        if (read(fileFD, &chain_block_no, sizeof(int)) == -1)
        {
            perror("read error\n");
            return 0;
        }
        printf("Block number %d in chain %d\n", chain_block_no, chno);
    }
    // off_t offRem = lseek(fileFD, offset, SEEK_SET);
    // if (offRem == (off_t)-1)
    // {
    //     perror("lseek error\n");
    //     return 0;
    // }
    offset = lseek(fileFD, 12 + (bno / 8) + chno * sizeof(int), SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    if (write(fileFD, &bno, sizeof(int)) == -1)
    {
        perror("write error\n");
        return 0;
    }
    offset = lseek(fileFD, 12 + (bno / 8) + chno * sizeof(int), SEEK_SET);
    int check;
    if (read(fileFD, &check, sizeof(int)) == -1)
    {
        perror("read error\n");
        return 0;
    }
    printf("The block is %d\n", check);
    offset = lseek(fileFD, 1024 + (chain_block_no)*blockSize, SEEK_SET);
    if (offset == (off_t)-1)
    {
        perror("lseek error\n");
        return 0;
    }
    if (write(fileFD, buffer, blockSize - 4) == -1)
    {
        perror("Error writing block size\n");
        return 0;
    }
    int next = -1;
    if (write(fileFD, &next, sizeof(int)) == -1)
    {
        perror("Error writing the next index\n");
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
            int bnoIn, chno;
            printf("Enter the chain number\n");
            scanf("%d", &chno);
            printf("Enter the block number\n");
            scanf("%d", &bnoIn);
            char *buffer = (char *)malloc((bsize - 4) * sizeof(char));
            if (read_block(argv[1], chno, bnoIn, buffer) == 0)
            {
                fprintf(stderr, "Reading block from file is unsuccessful\n");
                exit(EXIT_FAILURE);
            }
            printf("The buffer read is :\n %s\n", buffer);
            free(buffer);
        }
        else if (choice == 2)
        {
            int bnoIn, chno;
            printf("Enter the chain number\n");
            scanf("%d", &chno);
            printf("Enter the block number\n");
            scanf("%d", &bnoIn);
            char *buffer = (char *)malloc(sizeof(char) * (bsize - 4));
            int ch = rand() % 26;
            char pad = ('a' + ch);
            memset(buffer, pad, bsize);
            if (write_block(argv[1], chno, bnoIn, buffer) == 0)
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
