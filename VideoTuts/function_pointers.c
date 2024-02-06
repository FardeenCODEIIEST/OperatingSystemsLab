#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int sum(int x, int y)
{
	return x + y;
}

int prod(int x, int y)
{
	return x * y;
}


void shouldNotChange(int (*operation)(int, int))
{
	srand(time(NULL));
	int a = rand() % 100;
	int b = rand() % 100;
	printf("The result of the operation between %d and %d is %d\n", a, b, operation(a, b));
}

// Function pointer is used to chage objective without changing code of caller
int main(int argc, char *argv[])
{
	shouldNotChange(&prod);
	shouldNotChange(&sum);
	return 0;
}
