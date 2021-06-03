/* 	gcc -Wall -o EjePrintfCall EjePrintfCall.c 
	strace ./EjePrintfCall						*/
#include <stdio.h>
int main(void)
{ 
	printf("hello"); 
	return 0; 
}