#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
///#include "nonblock.h"
#include "portsimulator.h"
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "libterm.h"
//*****************Select Hardware Type: PI_Board or None (Simulaton)*********************************
#define NONE		0
#define PI_BOARD	1

#define HARDWARE 	PI_BOARD
//*****************************************************************************************



#define BIT_0 1<<0
#define ALL_BITS 0xFFFF

#define MOVELEFT           "\033[1D"
#define MOVE2TITLE         "\033[1;0H"
#define MOVE2PORT          "\033[3;0H"
#define MOVE2INSTRUCTIONS  "\033[6;0H"
#define MOVE2INPUT         "\033[15;0H"
#define ERASE              "\033[K"
#define SAVE_CURSOR        "\033[s"
#define RESTORE_CURSOR     "\033[u"
#define RED_TEXT		   "\e[31;1m"
#define BLACK_TEXT 		   "\e[30m"
#define WHITE_TEXT 		   "\e[37m"
#define CYAN_TEXT 		   "\e[36;1;52m"
#define GREEN_TEXT 		   "\e[32;1m"

#define OCHO_BITS 	8
#define DIECISEIS_BITS 16

void print_port(REG_t * registro,unsigned int port_size);
void auto_fantastico(REG_t * puerto);
void backspace (void);


typedef enum {EXIT,NOCHANGES,ON,OFF,TOGGLE,FLASH,ESC,PORT_A,PORT_B,PORT_D}status_t;


port_t active_port = PORTA;

int main(void)
{
	
	#if HARDWARE == PI_BOARD
	initspi();
	#endif
	
	system("clear");
	REG_t puerto;   // Creo el puerto
	puerto.portd=0; // Y lo pongo en 0

	status_t status = NOCHANGES;

	/*Imprimo el menu, con instrucciones de como usar el programa*/
	printf( MOVE2TITLE "Simulacion de registros"
		    MOVE2INSTRUCTIONS
			"(a) Seleccionar PORTA\n"
			"(b) Seleccionar PORTB\n"
			"(d) Seleccionar PORTD\n"
			"(s) Encender todos los bits del pueto activo\n"
			"(c) Apagar todos los bits del puerto activo\n"
			"(f) Parpadear los bits prendidos\n"
			"(h) Auto fantástico\n"
			"(1~8) Encender y apagar bit por bit PORTA y PORTB\n"
			"(1~8)(q~i) Encender y apagar bit por bit PORTD\n");
	fflush(stdout);
	print_port(&puerto,DIECISEIS_BITS);

    while(status!=EXIT)
    {
		/*Guardo el estado antes de llamar a menu*/
		/*Llamo al menu*/
		status=menu(&puerto);
		/*Para evitar glitchs visuales imprimo el puerto solo si el estado cambio*/
		if(status!=NOCHANGES)
			print_port(&puerto,DIECISEIS_BITS);	//status=NOCHANGES;

	}
	return 0;
}

int menu(REG_t * puerto)
{
	int status=NOCHANGES,c;
	unsigned int i;
	REG_t temp = *puerto;
	clock_t tiempo=clock();

	//nonblock(NB_ENABLE);
	if(kbhit())
	{
		backspace();
		switch (c=tolower(getchar()))
		{

			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
			{

				if(active_port==PORTA) c+=8;
				bitToggle(puerto,c-'0'-1);
				status=TOGGLE;

				break;
			}
			case'q':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D8);
				}
				status=TOGGLE;
				break;
			}
			case'w':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D9);
				}
				status=TOGGLE;
				break;
			}
			case'e':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D10);
				}
				status=TOGGLE;
				break;
			}
			case'r':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D11);
				}
				status=TOGGLE;
				break;
			}
			case't':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D12);
				}
				status=TOGGLE;
				break;
			}
			case'y':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D13);
				}
				status=TOGGLE;
				break;
			}
			case'u':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D14);
				}
				status=TOGGLE;
				break;
			}
			case'i':
			{
				if (active_port==PORTD)
				{
					bitToggle(puerto,D15);
				}
				status=TOGGLE;
				break;
			}
			case 'a':
			{
				active_port = PORTA;
				status=PORT_A;
				break;
			}
			case 'b':
			{
				active_port = PORTB;
				status = PORT_B;
				break;
			}
			case 'd':
			{
				active_port = PORTD;
				status = PORT_D;
				break;
			}
			case 'h':
			{
				auto_fantastico(puerto);
				break;
			}
			case 'f':
			{
				temp=*puerto;
				while(!kbhit())
				{
					if((((double)(clock()-tiempo))/(double)CLOCKS_PER_SEC)>=0.5)
					{
						tiempo=clock();
						temp.portd^=puerto->portd;
						#if HARDWARE == PI_BOARD
						spiWrite(get_porta(&temp));
						#endif
						//printf("pppp %.2X", get_porta(&temp));
						print_port(&temp,16);
						
					}
				}
				status=FLASH;
				break;
			}
			case 'c':
			{
				maskOff(puerto,active_port,ALL_BITS);
				status=OFF;
				break;
			}
			case 's':
			{
				maskOn(puerto,active_port,ALL_BITS);
				status=ON;
				break;
			}
			case 'z':
			{
				backspace();
				status=EXIT;
				break;
			}
			default:
				status=NOCHANGES;
		}
		#if HARDWARE == PI_BOARD
		spiWrite(get_porta(puerto));
		#endif
	}
	printf(MOVE2INPUT ERASE);
	fflush(stdout);
	////printf("pppp %.2X", get_porta(puerto));
	//nonblock(NB_DISABLE);  //no se usa mas 
	return status;
}


void print_port(REG_t * registro,unsigned int port_size)
{
	uint8_t bytes,bits;
	bit_t bit = D15;
	if(port_size%8);
	else
	{
		/* Imprime en verde el registro activo y en rojo los otros dos */
		if (active_port==PORTD)
			printf(SAVE_CURSOR MOVE2PORT
		  GREEN_TEXT"             PUERTO D\n"
		  RED_TEXT"    PUERTO A          PUERTO B\n"
		  WHITE_TEXT);

		else if (active_port==PORTA)
			printf(SAVE_CURSOR MOVE2PORT
		  RED_TEXT"             PUERTO D\n"
		  GREEN_TEXT"    PUERTO A"
		  RED_TEXT"          PUERTO B\n"
		  WHITE_TEXT);
		else
			printf(SAVE_CURSOR MOVE2PORT
		  RED_TEXT"             PUERTO D\n    PUERTO A          "
		  GREEN_TEXT"PUERTO B\n"
		  WHITE_TEXT);

		/* Separa el puerto en bytes */
		for(bytes=port_size/OCHO_BITS;bytes>0;bytes--)
		{
			/*Imprimo los bits de cada byte*/
			for (bits=OCHO_BITS;bits>0;bits--)
			{
				if(bitGet(registro,bit--))
					printf(GREEN_TEXT "⦿ " WHITE_TEXT);
				else
					printf(RED_TEXT "⦾ " WHITE_TEXT);
				fflush(stdout);
			}
			fputc(' ',stdout);
			fputc(' ',stdout);
		}
		printf(RESTORE_CURSOR);
		fflush(stdout);
	}
}

void auto_fantastico(REG_t * puerto)
{
	// Static para que si se oprime una tecla vuelva a comenzar de donde estaba
	static int i=30;
	clock_t tiempo = 0;
	while(!kbhit())
	{
		if( ( (double)( clock()-tiempo ) / (double)CLOCKS_PER_SEC ) >= 0.05 )
		{
			tiempo=clock();
			if (i%30<16)
				puerto->portd = 1<<i%30;
			else
				puerto->portd = 1<<30-i%30;
			i++;
			#if HARDWARE == PI_BOARD
			spiWrite(get_porta(puerto));
			#endif
			print_port(puerto,DIECISEIS_BITS);
		}
	}
	/*Tomo el input que hizo salir del while*/
	getchar();
}

void backspace (void)
{
	printf(MOVELEFT " " MOVELEFT);
	fflush(stdout);
}
