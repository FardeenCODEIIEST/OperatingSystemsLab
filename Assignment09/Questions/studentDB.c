#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_DESC_SIZE 500
#define MAX_NAME_SIZE 50

typedef struct
{
    int roll;
    char fname[MAX_NAME_SIZE];
    char mname[MAX_NAME_SIZE];
    char sname[MAX_NAME_SIZE];
    char desc[MAX_DESC_SIZE];
} studentRecord;

char *dataFile;
char *indexFile;

/*
    @brief:- Function to read an int value from the buffer
*/
int readIntFromBuffer(char **buffer)
{
    int value;
    memcpy(&value, *buffer, sizeof(int));
    *buffer += sizeof(int);
    return value;
}

/*
    @brief:- Function to read a string from the buffer
*/

void readStringFromBuffer(char **buffer, char *target, int size)
{
    memcpy(target, *buffer, size);
    *buffer += size;
    target[size] = '\0';
}

/*
    @brief:- Function to deserialise buffer into studentRecord
*/

studentRecord deserialiseBuffer(char *buffer)
{
    studentRecord st;

    st.roll = readIntFromBuffer(&buffer);
    readStringFromBuffer(&buffer, st.fname, 50);
    readStringFromBuffer(&buffer, st.mname, 50);
    readStringFromBuffer(&buffer, st.sname, 50);
    int descLen = readIntFromBuffer(&buffer);
    strncpy(st.desc, buffer, descLen);
    st.desc[descLen] = '\0';
    return st;
}

/*
    @brief:- Function to read the number of records in a file
*/
int readEntryCount(int fd)
{
    lseek(fd, 0, SEEK_SET);
    int count;
    read(fd, &count, sizeof(int));
    return count;
}

/*
    @brief:- Function updates the student record count
*/
void updateEntryCount(int fd)
{
    lseek(fd, 0, SEEK_SET); // Move to beginning of the file
    int n;
    read(fd, &n, sizeof(int));
    n++;
    lseek(fd, 0, SEEK_SET); // Move back to beginning to update
    write(fd, &n, sizeof(int));
}

/*
    @brief:- Function inserts student record in student.data file and also updates the student.index file
*/
void insertRecord(studentRecord *st)
{
    int dataFD = open(dataFile, O_RDWR | O_CREAT, 0666);
    int indexFD = open(indexFile, O_RDWR | O_CREAT, 0666);

    // Ensure initial count is written if files are new
    off_t dataEnd = lseek(dataFD, 0, SEEK_END);
    if (dataEnd == 0)
    {
        int initialCount = 0;
        write(dataFD, &initialCount, sizeof(int));
    }

    off_t indexEnd = lseek(indexFD, 0, SEEK_END);
    if (indexEnd == 0)
    {
        int initialCount = 0;
        write(indexFD, &initialCount, sizeof(int));
    }

    // Serialize the record
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    int offset = 0;

    memcpy(buffer + offset, &st->roll, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, st->fname, MAX_NAME_SIZE);
    offset += MAX_NAME_SIZE;
    memcpy(buffer + offset, st->mname, MAX_NAME_SIZE);
    offset += MAX_NAME_SIZE;
    memcpy(buffer + offset, st->sname, MAX_NAME_SIZE);
    offset += MAX_NAME_SIZE;
    int descLen = strlen(st->desc);
    memcpy(buffer + offset, &descLen, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, st->desc, descLen);
    offset += descLen;

    // Append record to data file
    off_t recordPosition = lseek(dataFD, 0, SEEK_END);

    // Update data file
    write(dataFD, buffer, offset);

    // Update index file
    write(indexFD, &recordPosition, sizeof(off_t));

    // Update counts in both files
    updateEntryCount(dataFD);
    updateEntryCount(indexFD);

    free(buffer);
    close(dataFD);
    close(indexFD);
}

/*
    @brief:- Function to search for a student record in the data file using the index file
*/
studentRecord searchRecord(int roll, int *flag)
{
    int indexFD = open(indexFile, O_RDONLY);
    if (indexFD == -1)
    {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }
    int dataFD = open(dataFile, O_RDONLY);
    if (dataFD == -1)
    {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }

    int n = readEntryCount(indexFD); // Number of entries
    *flag = 0;                       // Assume not found initially

    studentRecord st;
    memset(&st, 0, sizeof(st)); // Initialize to zero

    for (int i = 0; i < n; i++)
    {
        off_t offset;
        // Read the offset from the index file
        if (read(indexFD, &offset, sizeof(off_t)) != sizeof(off_t))
        {
            perror("Failed to read offset from index file");
            exit(EXIT_FAILURE);
        }

        // Move to the position in the data file
        if (lseek(dataFD, offset, SEEK_SET) == (off_t)-1)
        {
            perror("Failed to seek in data file");
            exit(EXIT_FAILURE);
        }

        // Read the serialized data size first (if you have such a mechanism in place)
        // For simplicity, I'm directly reading into a sufficiently large buffer here
        // char buffer[1024];                                    // Adjust size accordingly
        char *buffer = (char *)malloc(sizeof(char) * 1024);
        int bytesRead = read(dataFD, buffer, sizeof(char) * 1024); // Read the serialized record
        if (bytesRead == -1)
        {
            perror("Failed to read data");
            exit(EXIT_FAILURE);
        }

        // Deserialize the buffer into a studentRecord
        char *tempBuffer = buffer;
        st = deserialiseBuffer(tempBuffer);

        if (st.roll == roll)
        {
            *flag = 1; // Found
            break;
        }
        free(buffer);
    }

    close(indexFD);
    close(dataFD);
    return st;
}

/*
    @brief:- Function performs compaction of data in the student.data and student.index file to remove holes, saving memory space
*/

void compactData(int indexFD, int dataFD, int deleteIndex, int totalEntries)
{
    studentRecord st;
    off_t newIndex[totalEntries - 1], newOffset = sizeof(int); // Exclude deleted entry
    int tempDataFD = open("tempData.data", O_RDWR | O_CREAT | O_TRUNC, 0666);
    int tempIndexFD = open("tempIndex.index", O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (tempDataFD == -1 || tempIndexFD == -1)
    {
        perror("Creating temporary files failed");
        exit(EXIT_FAILURE);
    }

    // Rewriting the entry count minus one (as we're deleting one)
    int newCount = totalEntries - 1;
    write(tempDataFD, &newCount, sizeof(int));
    write(tempIndexFD, &newCount, sizeof(int));

    lseek(dataFD, sizeof(int), SEEK_SET);  // Skip past entry count
    lseek(indexFD, sizeof(int), SEEK_SET); // Skip past entry count

    for (int i = 0; i < totalEntries; i++)
    {
        off_t offset;
        read(indexFD, &offset, sizeof(off_t));

        // skip the deleted entries
        if (i == deleteIndex)
            continue; // Skip deleted entry

        lseek(dataFD, offset, SEEK_SET);
        read(dataFD, &st, sizeof(studentRecord));

        write(tempDataFD, &st, sizeof(studentRecord));     // Write to temp data file
        newIndex[i < deleteIndex ? i : i - 1] = newOffset; // Adjust index
        newOffset += sizeof(studentRecord);
    }

    // Write new indexes
    for (int i = 0; i < newCount; i++)
    {
        write(tempIndexFD, &newIndex[i], sizeof(off_t));
    }

    // Replace old files with new compacted ones
    rename("tempData.data", dataFile);
    rename("tempIndex.index", indexFile);

    close(tempDataFD);
    close(tempIndexFD);
}

/*
    @brief:- Function deletes the student record and also the corresponding pointer entry in the student.index file
*/

int deleteRecord(int roll)
{
    int flag = 0, indexFD, dataFD, n;
    off_t offset;
    studentRecord st;

    indexFD = open(indexFile, O_RDWR);
    dataFD = open(dataFile, O_RDWR);

    if (indexFD == -1 || dataFD == -1)
    {
        perror("Opening file failed");
        exit(EXIT_FAILURE);
    }

    n = readEntryCount(indexFD); // Get total entries

    for (int i = 0; i < n; i++)
    {
        read(indexFD, &offset, sizeof(off_t)); // Read index to find the record

        lseek(dataFD, offset, SEEK_SET);          // Seek to record position
        read(dataFD, &st, sizeof(studentRecord)); // Read record

        if (st.roll == roll)
        {
            flag = 1; // Record found
            // Compact data
            compactData(indexFD, dataFD, i, n);
            break;
        }
    }

    close(indexFD);
    close(dataFD);
    return flag;
}

/*
    @brief:- Function modifies a student record by first searching through rolls and then changing the record
*/
int modifyRecord(int roll, studentRecord *newRecord)
{
    // Delete followed by insert
    if (deleteRecord(roll) == 0)
    {
        return 0;
    }
    insertRecord(newRecord);
    return 1;
}

int main(int argc, char *argv[])
{
    // Usage :- [exe_name] [path of data file] [path of index file]
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <path of data file> <path of index file>\n", argv[0]);
        return 0;
    }
    dataFile = argv[1];
    indexFile = argv[2];
    // Take inputs from user
    while (1)
    {
        // compaction action is automatically done after each deletion operation
        printf("Enter the following options for the given actions :-\n 1. Insert Student Record\n 2. Search Student Record\n 3. Modify Student Record\n 4. Delete Student Record\n 5. Exit\n");
        int choice;
        scanf("%d", &choice);
        if (choice == 1)
        {
            // Insert
            studentRecord *st = (studentRecord *)malloc(sizeof(studentRecord));
            printf("Enter the roll number of student\n");
            scanf("%d", &st->roll);
            printf("Enter the first name of the student\n");
            scanf("%s", st->fname);
            st->fname[sizeof(st->fname) - 1] = '\0';
            printf("Enter the middle name of the student\n");
            scanf("%s", st->mname);
            st->mname[sizeof(st->mname) - 1] = '\0';
            printf("Enter the surname of the student\n");
            scanf("%s", st->sname);
            st->sname[sizeof(st->sname) - 1] = '\0';
            printf("Give a brief description of the student\n");
            getc(stdin);
            scanf("%[^\n]s", st->desc); // read until \n is encountered
            st->desc[sizeof(st->desc) - 1] = '\0';
            printf("The student details mentioned are:-\n");
            printf("Roll is:- %d\n", st->roll);
            printf("Name is:- %s %s %s\n", st->fname, st->mname, st->sname);
            printf("Description of the student is:-\n %s\n", st->desc);
            printf("Committing these details .........\n");
            insertRecord(st);
            printf("Record has been inserted\n");
            free(st);
        }
        else if (choice == 2)
        {
            // Search
            printf("Enter the roll-number\n");
            int roll, flag;
            scanf("%d", &roll);
            studentRecord st;
            printf("Searching started ....\n");
            st = searchRecord(roll, &flag);
            if (!flag)
            {
                printf("No such record exists\n");
                continue;
            }
            printf("The student details are:-\n");
            printf("Roll is:- %d\n", st.roll);
            printf("Name is:- %s %s %s\n", st.fname, st.mname, st.sname);
            printf("Description of the student is:-\n %s\n", st.desc);
        }
        else if (choice == 3)
        {
            // Modify
            printf("Enter the roll-number\n");
            int roll;
            scanf("%d", &roll);
            studentRecord *st = (studentRecord *)malloc(sizeof(studentRecord));
            printf("Give the new Details\n");
            printf("Enter the roll number of student\n");
            scanf("%d", &st->roll);
            printf("Enter the first name of the student\n");
            scanf("%s", st->fname);
            st->fname[sizeof(st->fname) - 1] = '\0';
            printf("Enter the middle name of the student\n");
            scanf("%s", st->mname);
            st->mname[sizeof(st->mname) - 1] = '\0';
            printf("Enter the surname of the student\n");
            scanf("%s", st->sname);
            st->sname[sizeof(st->sname) - 1] = '\0';
            printf("Give a brief description of the student\n");
            getc(stdin);
            scanf("%[^\n]s", st->desc); // read until \n is encountered
            st->desc[sizeof(st->desc) - 1] = '\0';
            printf("Modifying the details .....\n");
            if (modifyRecord(roll, st) == 0)
            {
                printf("Record cannot be modified\n");
                continue;
            }
            printf("Record has been modified\n");
        }
        else if (choice == 4)
        {
            // Delete
            int roll;
            printf("Enter the roll-number\n");
            scanf("%d", &roll);
            printf("Deleting the record ....\n");
            if (deleteRecord(roll) == 0)
            {
                printf("Record cannot be deleted\n");
                continue;
            }
            printf("Record has been deleted\n");
        }
        else if (choice == 5)
        {
            // Exit
            printf("Goodbye..... Thank you\n");
            break;
        }
        else
        {
            printf("Wrong choice Try again\n");
        }
    }
    return 0;
}
