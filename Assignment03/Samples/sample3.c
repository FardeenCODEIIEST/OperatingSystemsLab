#include <stdio.h>
#include <stdlib.h>

// Function to convert matrix to array of strings
char** matrixToCharArray(int m, int n, int matrix[m][n]) {
    // Allocate memory for array of pointers to char
    char** rows = (char**)malloc(m * sizeof(char*));

    // Iterate through each row of the matrix
    for (int i = 0; i < m; i++) {
        // Allocate memory for each row as a string
        rows[i] = (char*)malloc((n * 2 + 1) * sizeof(char)); // Each element can have 2 digits and a null terminator

        // Convert each element of the matrix to a character and store in the row string
        for (int j = 0; j < n; j++) {
            sprintf(rows[i] + j * 2, "%d ", matrix[i][j]); // Using sprintf to convert int to string
        }

        // Null-terminate the row string
        rows[i][n * 2 - 1] = '\0';
    }

    return rows;
}

// Function to free memory allocated for the array of strings
void freeCharArray(int m, char** rows) {
    for (int i = 0; i < m; i++) {
        free(rows[i]);
    }
    free(rows);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <number of rows> <number of columns>\n", argv[0]);
        return 1;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    int matrix[m][n];
    int ptr=3;
    // Input the matrix elements
    //printf("Enter the matrix elements:\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j]=atoi(argv[ptr++]);
        }
    }

    // Convert the matrix to an array of strings
    char** rows = matrixToCharArray(m, n, matrix);

    // Print the array of strings
    printf("Array of Strings:\n");
    for (int i = 0; i < m; i++) {
        printf("%s\n", rows[i]);
    }

    // Free allocated memory
    freeCharArray(m, rows);

    return 0;
}

