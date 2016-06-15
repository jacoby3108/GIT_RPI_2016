#include <stdio.h>
#include <stdint.h>
#include "portsimulator.h"
#define BIT_0 1<<0

void bitSet(REG_t * registro, bit_t bitnum)
{
	registro->portd |= 1<<bitnum;
}

void bitClear(REG_t * registro, bit_t bitnum)
{
	registro->portd &= (~(1<<bitnum));
}


void maskOn (REG_t * registro, port_t port, uint16_t mask)
{

	if (port == PORTD)
		registro->portd |= mask;

	if (port == PORTA)
		registro->port.a |= (uint8_t) mask;

	else if (port == PORTB)
	    registro->port.b |= (uint8_t)mask;
}

uint8_t get_porta(REG_t * registro)
{
	return registro->port.a;
}

uint8_t get_portb(REG_t * registro)
{
	return registro->port.b;
}

void maskOff (REG_t * registro, port_t port, uint16_t mask)
{

	if (port == PORTD)
		registro->portd &= (~mask);

	else if (port == PORTA)
		registro->port.a &= (uint8_t)(~mask);

	else if (port == PORTB)
	    registro->port.b &= (uint8_t)(~mask);
}


void maskToggle (REG_t * registro, port_t port, uint16_t mask)
{

	if (port == PORTD)
		registro->portd ^= mask;

	else if (port == PORTA)
		registro->port.a ^= (uint8_t)mask;

	else if (port == PORTB)
	    registro->port.b ^= (uint8_t)mask;
}

int bitGet (REG_t * registro, bit_t bit )
{
    return ((registro->portd>>bit) & BIT_0);
}

void bitToggle (REG_t * registro, bit_t bit )
{
    registro->portd ^= 1<<bit;
}

