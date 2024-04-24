#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>

typedef struct 
{
	int roll;
	char name[256];
	int marks;
}student;

void * func(void *args)
{
	student* st=(student*)args;
	student* res=(student*)malloc(sizeof(student));
	// change the name and roll
	res->roll=64;
	res->marks=st->marks;
	strcpy(res->name,"Wriddhiraj Dev");
	pthread_exit(res);
}


int main()
{
	pthread_t thread;
	// Initialise the student
	student* st=(student*)malloc(sizeof(student));
	st->roll=23;
	st->marks=100;// hopefully
	strcpy(st->name,"Sk Fardeen Hossain");
	int status=pthread_create(&thread,NULL,func,(void*)st);

	pthread_join(thread,(void**)&st); // st should be changed
	printf("The student details are:\n");
	printf("Name:- %s\n",st->name);
	printf("Roll:- %d\n",st->roll);
	printf("Marks:- %d\n",st->marks);
	return 0;
}
