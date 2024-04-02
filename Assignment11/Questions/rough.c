#include <stdio.h>     // for printf(),scanf(),...
#include <stdlib.h>    // for exit(), ...
#include <fcntl.h>     // for open(),...
#include <unistd.h>    // for read(),write(),lseek(),..
#include <sys/types.h> // for types ...
#include <string.h>    // for memset(),..
#include <time.h>      // for time(),..
#include <stdint.h>    // for standard interger types....
#include <stdbool.h>   //

#define METADATA_SIZE (sizeof(int) * 3) // For block size, total number of blocks, and free block count

int fileFD; // file descriptor

/*
    @brief:- Function initialises the file <fname> with its meta data bsize anf bno in the first 1024 bytes
    @param:- fileName: const char*, blockSize: int, Number of blocks:int
    @return:- On success return 0, else return -1

*/
int init_File_dd(const char *fname, int bsize, int bno) {
    fileFD = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fileFD == -1) {
        perror("Error opening file");
        return -1;
    }
    // Write metadata: block size, block number, number of free blocks
    if (write(fileFD, &bsize, sizeof(int)) == -1 || write(fileFD, &bno, sizeof(int)) == -1 || write(fileFD, &bno, sizeof(int)) == -1) {
        perror("Error writing metadata");
        close(fileFD);
        return -1;
    }
    // Initialize and write the bit pattern for block allocation
    int bytesNeeded = (bno + 7) / 8; // Number of bytes needed to represent all blocks
    uint8_t *pattern = calloc(bytesNeeded, sizeof(uint8_t));
    if (!pattern) {
        perror("Memory allocation failed");
        close(fileFD);
        return -1;
    }
    if (write(fileFD, pattern, bytesNeeded) == -1) {
        perror("Error initializing block allocation bit pattern");
        free(pattern);
        close(fileFD);
        return -1;
    }
    free(pattern);
    // Reserve space for block data (just by setting file size, data is not actually written)
    off_t fileSize = 12 + bytesNeeded + (long)bsize * bno;
    if (ftruncate(fileFD, fileSize) == -1) {
        perror("Error setting file size");
        close(fileFD);
        return -1;
    }
    close(fileFD);
    return 0;
}

/*
    @brief:- Function reads the <fname> file and reads the bno th block (0 indexed) from file and puts it into the buffer
    @param:- fileName: const char*, blockIndex: int, readBuffer: char*,chaintIndex:int
    @return:- On success return 1, else return 0
*/

int read_block(const char *fname, int bno, char *buffer, int bufSize) {
    int fd = open(fname, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 0;
    }

    int blockSize, totalBlocks, freeBlocks;
    // Read metadata: block size, total number of blocks, and free blocks count
    if (read(fd, &blockSize, sizeof(blockSize)) != sizeof(blockSize) ||
        read(fd, &totalBlocks, sizeof(totalBlocks)) != sizeof(totalBlocks) ||
        read(fd, &freeBlocks, sizeof(freeBlocks)) != sizeof(freeBlocks)) {
        perror("Failed to read metadata");
        close(fd);
        return 0;
    }

    if (bno < 0 || bno >= totalBlocks) {
        fprintf(stderr, "Invalid block number\n");
        close(fd);
        return 0;
    }

    // Calculate the number of bytes for the bitmap (1 bit per block)
    int bytesForBitmap = (totalBlocks + 7) / 8;

    // Calculate the offset to the desired block
    off_t offset = METADATA_SIZE + bytesForBitmap + (off_t)bno * blockSize;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Failed to seek to block");
        close(fd);
        return 0;
    }

    // Read the block data into the buffer
    if (read(fd, buffer, bufSize) != bufSize) {
        perror("Failed to read data from block");
        close(fd);
        return 0;
    }

    close(fd);
    return 1; // Success
}

/*
    @brief:- Function writes to the bno th block (0 indexed) in the <fname> file taking values from the buffer
    @param:- fileName: const char*, blockIndex: int, writeBuffer: char*, chainIndex:int
    @return:- On success return 1, else return 0
*/
int find_free_block(int fd, int totalBlocks, int bytesForBitmap) {
    lseek(fd, METADATA_SIZE, SEEK_SET); // Skip to the bitmap
    unsigned char *bitmap = malloc(bytesForBitmap);
    if (!bitmap) {
        perror("Failed to allocate memory for bitmap");
        return -1;
    }

    if (read(fd, bitmap, bytesForBitmap) != bytesForBitmap) {
        perror("Failed to read bitmap");
        free(bitmap);
        return -1;
    }

    int blockIndex = -1;
    for (int i = 0; i < totalBlocks; ++i) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) { // Check if the bit is 0 (free)
            blockIndex = i;
            bitmap[i / 8] |= (1 << (i % 8)); // Mark as used
            break;
        }
    }

    if (blockIndex != -1) {
        // Write back the updated bitmap
        lseek(fd, METADATA_SIZE, SEEK_SET);
        if (write(fd, bitmap, bytesForBitmap) != bytesForBitmap) {
            perror("Failed to write updated bitmap");
            free(bitmap);
            return -1;
        }
    }

    free(bitmap);
    return blockIndex; // -1 if no free block is found
}

int write_block(const char *fname, int bno, const char *buffer, int bufSize) {
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        return 0;
    }

    int blockSize, totalBlocks, freeBlocks;
    if (read(fd, &blockSize, sizeof(blockSize)) != sizeof(blockSize) ||
        read(fd, &totalBlocks, sizeof(totalBlocks)) != sizeof(totalBlocks) ||
        read(fd, &freeBlocks, sizeof(freeBlocks)) != sizeof(freeBlocks)) {
        perror("Failed to read metadata");
        close(fd);
        return 0;
    }

    int bytesForBitmap = (totalBlocks + 7) / 8;

    if (bno == -1) { // Allocate a new block if bno is -1
        bno = find_free_block(fd, totalBlocks, bytesForBitmap);
        if (bno == -1) {
            fprintf(stderr, "No free blocks available\n");
            close(fd);
            return 0;
        }
    } else {
        // Additional validation can be added here to check if the block number is valid
    }

    off_t offset = METADATA_SIZE + bytesForBitmap + (off_t)bno * blockSize;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Failed to seek to block");
        close(fd);
        return 0;
    }

    if (write(fd, buffer, bufSize) != bufSize) {
        perror("Failed to write data to block");
        close(fd);
        return 0;
    }

    close(fd);
    return 1; // Success
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
