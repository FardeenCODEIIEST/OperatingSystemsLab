#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to split the command line arguments into parts of three
char** splitArgumentsIntoPartsOfThree(int argc, char* argv[]) {
    // Calculate the number of substrings needed
    int numSubstrings = argc - 1;

    // Allocate memory for array of pointers to char
    char** result = (char**)malloc(numSubstrings * sizeof(char*));

    // Iterate through each argument
    for (int i = 1; i < argc; i++) {
        // Allocate memory for each substring
        result[i - 1] = (char*)malloc(4 * sizeof(char)); // Each part can have 3 digits and a space

        // Copy 3 characters (or less if at the end of the string)
        strncpy(result[i - 1], argv[i], 3);

        // Null-terminate the substring
        result[i - 1][3] = '\0';
    }

    return result;
}

// Function to free memory allocated for the array of strings
void freeStringArray(int numStrings, char** array) {
    for (int i = 0; i < numStrings; i++) {
        free(array[i]);
    }
    free(array);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number1> <number2> <number3> ...\n", argv[0]);
        return 1;
    }

    // Split the command line arguments into parts of three
    char** result = splitArgumentsIntoPartsOfThree(argc, argv);

    // Print the array of strings
    printf("Result:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%s ", result[i]);
    }
    printf("\n");

    // Free allocated memory
    freeStringArray(argc - 1, result);

    return 0;
}
	
