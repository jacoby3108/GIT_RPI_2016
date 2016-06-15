#ifndef portsimulator_h
#define portsimulator_h

typedef int16_t WORD_REG;

typedef struct{
	int8_t b;
	int8_t a;
}BYTE_REG;

typedef	struct{
	int16_t b0:1;
	int16_t b1:1;
	int16_t b2:1;
	int16_t b3:1;
	int16_t b4:1;
	int16_t b5:1;
	int16_t b6:1;
	int16_t b7:1;
	int16_t b8:1;
	int16_t b9:1;
	int16_t b10:1;
	int16_t b11:1;
	int16_t b12:1;
	int16_t b13:1;
	int16_t b14:1;
	int16_t b15:1;
}BIT_REG_16;

typedef	struct{
	int8_t b0:1;
	int8_t b1:1;
	int8_t b2:1;
	int8_t b3:1;
	int8_t b4:1;
	int8_t b5:1;
	int8_t b6:1;
	int8_t b7:1;
}BIT_REG_8;

typedef struct {
	BIT_REG_8  b;
	BIT_REG_8  a;
}BIT_PORTAB;

typedef union {
	WORD_REG    portd;		// PORTD como un int16_t (.portd)
	BYTE_REG    port;		// PORTA y PORTB como un int8_t (.port.a - .port.b)
	BIT_REG_16  bit_portd;	// PORTD bit a bit (bit_portd .b0~.b15)
	BIT_PORTAB  bit_portab;	// PORTA y PORTB bit a bit (bit_portab.a/.b .b0~.b7)
}REG_t;


typedef enum {B0,B1,B2,B3,B4,B5,B6,B7,A0,A1,A2,A3,A4,A5,A6,A7,
			  D0=0,D1,D2,D3,D4,D5,D6,D7,D8,D9,D10,D11,D12,D13,D14,D15}bit_t;

typedef enum {PORTD,PORTB,PORTA}port_t;

/* Recibe un puntero al registro y un bit_t. Devuelve el estado de ese bit */
int  bitGet (REG_t * registro, bit_t bit );

/* Recibe un puntero al registro y un bit_t. Cambia el estado de ese bit*/
void bitToggle (REG_t * registro, bit_t bit );

/* Recibe un puntero al registro y un bit_t. Prende ese bit*/
void bitSet(REG_t * registro, bit_t bitnum);

/* Recibe un puntero al registro y un bit_t. Apaga ese bit*/
void bitClear(REG_t * registro, bit_t bitnum);

/* Reciben un puntero al registro, un puerto_t y una mascara.
Aplican la mascara al puerto solicitado*/
void maskOn (REG_t * registro, port_t port, uint16_t mask);
void maskOff (REG_t * registro, port_t port, uint16_t mask);
void maskToggle (REG_t * registro, port_t port, uint16_t mask);

uint8_t get_porta(REG_t * registro);

uint8_t get_portb(REG_t * registro);

#endif // portsimulator_h
