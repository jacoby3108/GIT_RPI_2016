#include <stdio.h>
#include <string.h>

/*strignificacion*/

#define TEXTIFY(A) #A

#define CONCAT(value)  TEXTIFY(Begin_ ## value ## _END)
#define DIR(pin) TEXTIFY(/sys/class/gpio/gpio ## pin ## _direction)
#define DIR1(pin) TEXTIFY(/sys/class/gpio/gpio ## pin )

char temp[100];


void main (void)
{
	printf("Result:::%s\n",TEXTIFY(17));
	printf("Result:::%s\n",CONCAT(17));
	printf("Result:::%s\n",DIR(17));
	printf("Result:::%s\n",DIR1(4));
	
	
	strcpy(temp,DIR1(4));
	strcat(temp,"/direction");
		
	printf("Temp:::%s\n",temp);
}
