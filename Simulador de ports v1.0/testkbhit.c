#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "libterm.h"
char c;

int main(void)
{
	
	c=getch();
	
	printf(" getch without echo character was: %c \n",c);
	
	c=getche();
	
	printf(" getch with echo character was: %c \n",c);
	
	puts("Press a key!");
	while(!kbhit());
	
    
	printf("You pressed '%c'!\n", getchar());
	return 0;
}
