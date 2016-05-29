#include <stdio.h>
#include <string.h>

/*strignificacion*/
#define GPIO  /sys/class/gpio/gpio
#define DIR   _direction

#define TEXTIFY(A) #A

#define CONCAT(A,B) CONCAT_(A,B) 
#define CONCAT_(A,B) A##B


void main (void)
{
	printf("Result:::%s\n",CONCAT(GPIO,DIR));
	
	
}
