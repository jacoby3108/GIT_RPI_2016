#include <stdio.h>
FILE *fp;

int c;
int i;
char msg[]="10";


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
	printf("-%c (%.2X)-\n",(char)c,(char)c);

	while ((c=fgetc(fp))!=EOF)
	putchar(c);


/// TX

	i=0;

	while(msg[i])
	fputc((int)msg[i++],fp);

	fputc(0x00,fp);
	 
	fclose(fp);

	return(0);

}
