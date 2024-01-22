#include<unistd.h>

/*
 *  0 --> stdin
 *  1 --> stdout
 *  2 --> stderr
 */
void main(){
	write(1,"Write works\n",12);
}
