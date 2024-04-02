#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 100
#define MAX_COMMANDS 100
#define MAX_OPTIONS 10

typedef struct
{
    char *command[MAX_COMMANDS];
    int index;
    int size;
} commands;

void executeCommand(char *args)
{
    pid_t pid = fork();
    int status;
    if (pid == -1)
    {
        perror("fork error\n");
        exit(EXIT_FAILURE);
    }
    char *argList[MAX_OPTIONS];
    char *tok;
    int c = 0;
    while ((tok = strsep(&args, " ")))
    {
        argList[c] = (char *)malloc(sizeof(char) * 20);
        strncpy(argList[c], tok, strlen(tok));
        c++;
    }
    argList[c - 1][strlen(argList[c - 1])] = '\0';
    argList[c] = (char *)0;
    if (pid == 0)
    {
        // child
        char binFile[5] = "/bin/";
        strcat(binFile, argList[0]);
        strcpy(argList[0], binFile);
        status = execvp(argList[0], argList);
        if (status == -1)
        {
            perror("execve failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // parent
        pid_t cpid = wait(&status);
        return;
    }
}

int main(int argc, char *argv[])
{
    printf("-------------------------- Welcome to MYSHELL ------------------------------\n");
    printf("Multiple Commands along wth options must be space separated\n");
    commands cmd;
    cmd.index = -1;
    cmd.size = 0;
    while (1)
    {
        char *input_buffer = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
        printf("shell> ");
        // getchar();
        fgets(input_buffer, MAX_BUFFER_SIZE, stdin);
        input_buffer[strlen(input_buffer)] = '\0';
        printf("The Command entered is %s\n", input_buffer);
        // sleep(1);
        bool andFlag = false;
        bool orFlag = false;
        char sep;
        for (int i = 0; i < strlen(input_buffer); i++)
        {
            if (input_buffer[i] == '&' || input_buffer[i] == ';')
            {
                sep = input_buffer[i];
                andFlag = true;
            }
            else if (input_buffer[i] == '|')
            {
                orFlag = true;
            }
        }
        if (andFlag == false && orFlag == false)
        {
            // printf("The single command is %s\n", input_buffer);
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;
            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            strcpy(cmd.command[cmd.index], input_buffer);
            if (strncmp(input_buffer, "exit", 4) == 0)
            {
                printf("GoodBye From shell :)\n");
                exit(EXIT_SUCCESS);
            }
            executeCommand(input_buffer);
            printf("Commands used till now are:-\n");
            cmd.command[cmd.index][strlen(cmd.command[cmd.index])] = '\0';
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
            continue;
        }
        char *command_copy, *command_free;
        command_copy = command_free = strdup(input_buffer); // copy the command
        char *token;
        int counter = 0;
        if (andFlag)
        {
            while ((token = strsep(&command_copy, &sep)))
            {
                if (counter & 1)
                {
                    counter++;
                    continue;
                }
                cmd.index = (cmd.index + 1) % MAX_COMMANDS;
                cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
                printf("Token is %s\n", token);
                if (strncmp(token, "exit", 4) == 0)
                {
                    printf("GoodBye From shell :)\n");
                    exit(EXIT_SUCCESS);
                }
                cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
                strcpy(cmd.command[cmd.index], token);
                cmd.command[cmd.index][strlen(cmd.command[cmd.index])] = '\0';
                counter++;
            }
            printf("Commands used till now are:-\n");
            for (int i = 0; i < cmd.size; i++)
            {
                printf("%s\n", cmd.command[i]);
            }
        }
        else
        {
            token = strsep(&command_copy, "|");
            if (counter & 1)
            {
                counter++;
                continue;
            }
            cmd.index = (cmd.index + 1) % MAX_COMMANDS;
            cmd.size = (cmd.size == MAX_COMMANDS) ? MAX_COMMANDS : (cmd.size + 1);
            printf("Token is %s\n", token);
            if (strncmp(token, "exit", 4) == 0)
            {
                printf("GoodBye From shell :)\n");
                exit(EXIT_SUCCESS);
            }
            cmd.command[cmd.index] = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
            strcpy(cmd.command[cmd.index], token);
            cmd.command[cmd.index][strlen(cmd.command[cmd.index])] = '\0';
            counter++;

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

// [/bin/ls] ls -lt