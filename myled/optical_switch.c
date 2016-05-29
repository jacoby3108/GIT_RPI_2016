#include <stdio.h>
FILE *fp;

int c;


int main(void)
{
	fp=fopen("/dev/rfbb", "r+");
	
	if (fp == NULL) /* siempre se debe hacer esta comprobación*/
        {
            puts ("No se puede abrir fichero.");
            return (1);
        }
		 
	puts("Se recibio:::");
	while ((c=fgetc(fp))!=EOF)
	printf("%c",c);

//	printf("-%X-",c);
		
		
//	putchar('\n');	


	fclose(fp);

	return(0);

}
