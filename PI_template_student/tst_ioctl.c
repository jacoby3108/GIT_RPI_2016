#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "query_ioctl.h"

 


void main (void)
{
int  archivo;
query_arg_t q;

archivo=open("/dev/rfbb", O_RDWR);

	q.param1=0;    //set something
	q.param2=0;	   //set something (will be modified by dirver)
	q.param3=0;	   //set something (will be modified by dirver)
	
	
	printf("Before call param1: %d  param2: %d param3: %d  \n\n", q.param1,q.param2,q.param3);

	if (ioctl(archivo, QUERY_GET_VARIABLES, &q) == -1)		// Get initial state of driver parameters
    {
        perror("query_apps ioctl get");
    }
	
	printf("Actual driver parameters param1: %d  param2: %d param3: %d  \n\n", q.param1,q.param2,q.param3);
	
	
	q.param1=100;  //set new value on driver 

	printf("Changing param1 to:%d \n\n", q.param1);

    if (ioctl(archivo, QUERY_SET_VARIABLES, &q) == -1)		// set new data (param1)
    {
        perror("query_apps ioctl set");
    }
 
    if (ioctl(archivo, QUERY_GET_VARIABLES, &q) == -1)		// then read back data from driver
    {
        perror("query_apps ioctl get");
    }
    
    
        printf("After set param1: %d  param2: %d param3: %d  \n\n", q.param1,q.param2,q.param3);

    


close(archivo);

}
