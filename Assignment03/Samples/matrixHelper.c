// multiply.c

#include <stdio.h>

int multiply(int n, int row[n], int col[n]) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (row[i] * col[i]);
    }
    return sum;
}

int main() {
    // This main function is here just for compilation purposes,
    // you won't be using it. The actual multiplication function is multiply.
    return 0;
}

