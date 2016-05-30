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

	q.data_step=1;
	

    if (ioctl(archivo, QUERY_SET_VARIABLES, &q) == -1)
    {
        perror("query_apps ioctl set");
    }
 
    if (ioctl(archivo, QUERY_GET_VARIABLES, &q) == -1)
    {
        perror("query_apps ioctl get");
    }
    else
    {

       printf("data_step : %d\n", q.data_step);

    }


close(archivo);

}
