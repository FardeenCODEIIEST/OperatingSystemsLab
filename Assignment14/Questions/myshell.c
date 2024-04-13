#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 100
#define MAX_COMMANDS 100
#define MAX_OPTIONS 10

typedef struct
{
    char *command[MAX_COMMANDS];
    int index;
    int size;
} commands;

// Utility Functions

char *removeLeading(char *str, char *str1)
{
    int idx = 0, j, k = 0;
    str1 = strdup(str);
    // Iterate String until last
    // leading space character
    while (str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n')
    {
        idx++;
    }

    // Run a for loop from index until the original
    // string ends and copy the content of str to str1
    for (j = idx; str[j] != '\0'; j++)
    {
        str1[k] = str[j];
        k++;
    }

    // Insert a string terminating character
    // at the end of new string
    str1[k] = '\0';
    return str1;
}

char *removeTrailing(char *str, char *str1)
{
    str1 = strdup(str);
    int idx = strlen(str) - 1, j, k = 0;

    // Iterate String until last
    // trailing space character
    while (str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n')
    {
        idx--;
    }

    // Run a for loop from index until the original
    // string ends and copy the content of str to str1
    k = idx;
    for (j = idx; j >= 0; j--)
    {
        str1[k] = str[j];
        k--;
    }
    return str1;
}

// cd must be done in the parent process itself
int executeCommand(char *args)
{
    pid_t pid = fork();
    int status;
    if (pid == -1)
    {
        perror("fork error\n");
        return -1;
    }
    if (pid == 0)
    {
        // child
        char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
        int c = 0;
        char *token = strtok(args, " ");
        while (token != NULL && c < MAX_OPTIONS)
        {
            argList[c] = strdup(token); // Allocate and copy
            token = strtok(NULL, " ");
            c++;
        }
        argList[c] = NULL; // execvp expects a NULL terminated array

        // No need to modify the path, execvp will search in PATH environment variable
        if (execvp(argList[0], argList) == -1)
        {
            perror("execvp failed\n");
            return -1;
        }
    }
    else
    {
        // parent
        wait(&status);
        return 0;
    }
}

int executeCommandWithRedirection(char *args)
{
    char *commandPart = strtok(args, ">");
    char *filePart = strtok(NULL, "");

    if (commandPart == NULL || filePart == NULL)
    {
        fprintf(stderr, "Redirection syntax error\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork error");
        return -1;
    }

    if (pid == 0)
    { // Child process
        // Trim leading and trailing spaces from file part
        while (isspace((unsigned char)*filePart))
            filePart++;
        char *end = filePart + strlen(filePart) - 1;
        while (end > filePart && isspace((unsigned char)*end))
            end--;
        *(end + 1) = 0;

        int fd = open(filePart, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1)
        {
            perror("open error");
            exit(EXIT_FAILURE);
        }

        // Redirect stdout to the file
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 error");
            exit(EXIT_FAILURE);
        }
        // child
        char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
        int c = 0;
        char *token = strtok(commandPart, " ");
        while (token != NULL && c < MAX_OPTIONS)
        {
            argList[c] = strdup(token); // Allocate and copy
            token = strtok(NULL, " ");
            c++;
        }
        argList[c] = NULL; // execvp expects a NULL terminated array

        // No need to modify the path, execvp will search in PATH environment variable
        if (execvp(argList[0], argList) == -1)
        {
            perror("execvp failed\n");
            return -1;
        }
        close(fd);
    }
    else
    { // Parent process
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}

int executeCommandWithPipe(char *input, int pipeCount)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe error");
        return -1;
    }

    char *command1 = strtok(input, "|");
    char *command2 = strtok(NULL, "");

    if (command1 == NULL || command2 == NULL)
    {
        fprintf(stderr, "Pipe syntax error\n");
        return -1;
    }
    pid_t pidArray[pipeCount];
    int index = 0;
    // while (pipeCount--)
    // {
    pidArray[index] = fork();
    if (pidArray[index] == 0)
    { // First child for the first command
        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        // Execute the first command (similar logic to executeCommand)
        char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
        int c = 0;
        char *token = strtok(command1, " ");
        while (token != NULL && c < MAX_OPTIONS)
        {
            argList[c] = strdup(token); // Allocate and copy
            token = strtok(NULL, " ");
            c++;
        }
        argList[c] = NULL; // execvp expects a NULL terminated array

        // No need to modify the path, execvp will search in PATH environment variable
        if (execvp(argList[0], argList) == -1)
        {
            perror("execvp failed\n");
            return -1;
        }
    }

    pidArray[index + 1] = fork();
    if (pidArray[index + 1] == 0)
    { // Second child for the second command
        // Redirect stdin to the read end of the pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);

        // Execute the second command (similar logic to executeCommand)
        char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
        int c = 0;
        char *token = strtok(command2, " ");
        while (token != NULL && c < MAX_OPTIONS)
        {
            argList[c] = strdup(token); // Allocate and copy
            token = strtok(NULL, " ");
            c++;
        }
        argList[c] = NULL; // execvp expects a NULL terminated array

        // No need to modify the path, execvp will search in PATH environment variable
        if (execvp(argList[0], argList) == -1)
        {
            perror("execvp failed\n");
            return -1;
        }
    }
    index++;
    // }

    // Close the pipe in the parent and wait for both children
    close(pipefd[0]);
    close(pipefd[1]);
    for (int i = 0; i <= pipeCount; i++)
    {
        waitpid(pidArray[i], NULL, 0);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc >= 2)
    {
        int fp_index = 1;
        while (fp_index < argc)
        {
            printf("Current script file is :%s\n", argv[fp_index]);
            FILE *fp = fopen(argv[fp_index], "rb");
            if (fp == NULL)
            {
                perror("fp error\n");
                return 0;
            }
            char *input_buffer = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            memset(input_buffer, 0, sizeof(char) * MAX_BUFFER_SIZE);
            while (fgets(input_buffer, MAX_BUFFER_SIZE, fp) != NULL)
            {
                if (input_buffer[strlen(input_buffer) - 1] == '\n')
                    input_buffer[strlen(input_buffer) - 1] = '\0'; // Remove newline
                if (strncmp(input_buffer, "exit", 4) == 0)
                {
                    printf("GoodBye From shell :)\n");
                    exit(EXIT_SUCCESS);
                }
                else if (strncmp(input_buffer, "cd", 2) == 0)
                {
                    char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
                    int c = 0;
                    char *token = strtok(input_buffer, " ");
                    while (token != NULL && c < MAX_OPTIONS)
                    {
                        argList[c] = strdup(token); // Allocate and copy
                        token = strtok(NULL, " ");
                        c++;
                    }
                    argList[c] = NULL; // execvp expects a NULL terminated array
                    printf("Changing directory to : %s\n", argList[1]);
                    if (chdir(argList[1]) == -1)
                    {
                        perror("chdir error:\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("Current path is :\n");
                    executeCommand("pwd");
                }
                else
                    executeCommand(input_buffer);
            }
            free(input_buffer);
            fclose(fp);
            // After all is executed shell needs to go to its place
            printf("Current path is :\n");
            executeCommand("pwd");
            if (chdir("/home/fardeen/Desktop/OSLab/Assignment13/Questions/") == -1)
            {
                perror("chdir error\n");
                return 0;
            }
            fp_index++;
        }
        return 0;
    }
    printf("-------------------------- Welcome to MYSHELL ------------------------------\n");
    // printf("Multiple Commands along wth options must be space separated\n");
    commands cmd;
    cmd.index = -1;
    cmd.size = 0;
    int user_pointer = -1;
    while (1)
    {
        char *input_buffer = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
        memset(input_buffer, 0, sizeof(char) * MAX_BUFFER_SIZE);
        printf("shell> ");
        // getchar();
        fgets(input_buffer, MAX_BUFFER_SIZE, stdin);
        if (input_buffer[strlen(input_buffer) - 1] == '\n')
            input_buffer[strlen(input_buffer) - 1] = '\0'; // Remove newline
        printf("The Command entered is %s\n", input_buffer);
        bool andFlag = false;
        bool orFlag = false;
        bool pipeFlag = false;
        bool redirectFlag = false;
        int pipeCount = 0;
        char sep;
        for (int i = 0; i < strlen(input_buffer); i++)
        {
            if (input_buffer[i] == '&' || input_buffer[i] == ';')
            {
                sep = input_buffer[i];
                andFlag = true;
            }
            else if (input_buffer[i] == '|' && input_buffer[i + 1] == '|')
            {
                orFlag = true;
            }
            else if (input_buffer[i] == '|' && input_buffer[i + 1] != '|')
            {
                orFlag = false;
                pipeFlag = true;
                pipeCount++;
            }
            else if (input_buffer[i] == '>')
            {
                redirectFlag = true;
            }
        }
        if (andFlag == false && orFlag == false && !pipeFlag && !redirectFlag)
        {
            // printf("The single command is %s\n", input_buffer);
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;
            user_pointer = cmd.index;
            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            int len = strlen(input_buffer);
            strncpy(cmd.command[cmd.index], input_buffer, len);
            if (strncmp(cmd.command[cmd.index], "exit", 4) == 0)
            {
                printf("GoodBye From shell :)\n");
                exit(EXIT_SUCCESS);
            }
            else if (strncmp(cmd.command[cmd.index], "cd", 2) == 0)
            {
                char *argList[MAX_OPTIONS + 1]; // +1 for the NULL at the end
                int c = 0;
                char *token = strtok(input_buffer, " ");
                while (token != NULL && c < MAX_OPTIONS)
                {
                    argList[c] = strdup(token); // Allocate and copy
                    token = strtok(NULL, " ");
                    c++;
                }
                argList[c] = NULL; // execvp expects a NULL terminated array
                printf("Changing directory to : %s\n", argList[1]);
                if (chdir(argList[1]) == -1)
                {
                    perror("chdir error:\n");
                    exit(EXIT_FAILURE);
                }
                printf("Current path is :\n");
                executeCommand("pwd");
                printf("Commands used till now are:-\n");
                for (int i = 0; i < cmd.size; i++)
                {
                    printf("%s\n", cmd.command[i]);
                }
            }
            else
            {
                executeCommand(input_buffer);
                printf("Commands used till now are:-\n");
                for (int i = 0; i < cmd.size; i++)
                {
                    printf("%s\n", cmd.command[i]);
                }
            }
        }
        char *command_copy, *command_free;
        command_copy = command_free = strdup(input_buffer); // copy the command
        char *token;
        int counter = 0;
        if (andFlag)
        {
            printf("AND\n");
            while ((token = strsep(&command_copy, &sep)))
            {
                if (counter & 1)
                {
                    counter++;
                    continue;
                }
                cmd.index = (cmd.index + 1) % MAX_COMMANDS;
                user_pointer = cmd.index;

                cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
                printf("Token is %s\n", token);
                if (strncmp(token, "exit", 4) == 0)
                {
                    printf("GoodBye From shell :)\n");
                    exit(EXIT_SUCCESS);
                }
                cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
                strcpy(cmd.command[cmd.index], token);
                // Execute the command
                executeCommand(cmd.command[cmd.index]);

                counter++;
            }
            printf("Commands used till now are:-\n");
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
        }
        else if (pipeFlag)
        {
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;

            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            strcpy(cmd.command[cmd.index], input_buffer);
            printf("PIPE\n");
            executeCommandWithPipe(input_buffer, pipeCount);
            printf("Commands used till now are:-\n");
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
        }
        else if (orFlag)
        {
            printf("OR\n");
            token = strsep(&command_copy, "|");
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;
            user_pointer = cmd.index;

            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            // printf("Token is %s\n", token);
            if (strncmp(token, "exit", 4) == 0)
            {
                printf("GoodBye From shell :)\n");
                exit(EXIT_SUCCESS);
            }
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            strcpy(cmd.command[cmd.index], token);
            // execute the command
            executeCommand(cmd.command[cmd.index]);

            printf("Commands used till now are:-\n");
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
        }
        else if (redirectFlag)
        {
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;

            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            strcpy(cmd.command[cmd.index], input_buffer);

            executeCommandWithRedirection(input_buffer);
            printf("Commands used till now are:-\n");
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
        }
        free(input_buffer);
    }
    return 0;
}
