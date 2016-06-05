/*
 * termlib.h
 *
 *  Created on: 2/06/2016
 *  Author: Daniel Jacoby
 *  Tested on x86 and pi board 
 *  See testkbhit.c for examples
 */
#ifndef TERMLIB_H_
#define TERMLIB_H_


/* Linux support functions for getch,getche and kbhit*/

char getch(void);			// Wait for keystroke (no echo)
char getche(void);			// Wait for keystroke (with echo)
int  kbhit(void);			// Signal if key is awaiting on keyboard buffer (returns 1 if something on kb buffer)

#endif /* TERMLIB_H_ */
