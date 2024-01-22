#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

int multiply(int n, int row[n], int col[n]) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (row[i] * col[i]);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);
    int A[m][n];
    int B[n][r];
    int B_transpose[r][n];
    int product[m][r];
    int c = 4;

    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < r; j++) {
            B[i][j] = atoi(argv[c++]);
        }
    }

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < n; j++) {
            B_transpose[i][j] = B[j][i];
        }
    }

    pid_t pid;

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            pid = fork();

            if (pid == -1) {
                perror("fork failed:\n");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                
                int res = multiply(n, A[i], B_transpose[j]);
                exit((res << 16) | (i << 8) | j);
            }
        }
    }

    
    for (int i = 0; i < m * r; i++) {
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            int result = WEXITSTATUS(status);
            int res_value = result >> 16;
            int res_i = (result >> 8) & 0xFF;
            int res_j = result & 0xFF;
            product[res_i][res_j] = res_value;
        }
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            printf("%d ", product[i][j]);
        }
        printf("\n");
    }

    exit(0);
}

