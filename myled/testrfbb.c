#include <stdio.h>
FILE *fp;
char usrmsg[128];
char c;
char msg[]="hola lcd 123";
char *pmsg;

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
	putchar(c);


	pmsg=&msg[0];
	
	while(c=*pmsg++)
	fputc(c,fp);

	fclose(fp);

	return(0);

}
