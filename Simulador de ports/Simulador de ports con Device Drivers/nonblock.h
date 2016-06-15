#ifndef __NONBLOCK_H
#define  __NONBLOCK_H

/*
 * Library: nonblock
 * This library allows non blocking reading from stdin on Linux systems.
 * Author: Clementina Calvo
 * Date: 05/2016
 *
 * Heavily based on this blog post: http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
 */

typedef enum {NB_ENABLE, NB_DISABLE} nb_State;

/* int kbhit(void)
 * This function performs non-blocking checking on the standard input (stdin)
 * Input: -
 * Output: non-zero if there's user input on stdin, else zero.
 *
 * Warning: When in canonical mode, input won't be retrieved unless user hits 'enter'
 * afterwards.
 */

int kbhit(void);

/* Due to the canonical mode of your terminal, enter needs to be hit to confirm user input. 
 * Canonical mode means it always waits for enter to confirm the user input. 
 * If that is not your case, bellow is another function to cater that.
 *
 * void nonblock(nb_State state)
 * This functions enables/disables canonical mode on stdin.
 * Input: nb_State state, the desired action.
 * Output: -
 *
 * For more info on canonical mode: http://www.gnu.org/software/libc/manual/html_node/Canonical-or-Not.html
 */
void nonblock(nb_State state);


/* General usage:
 *
 * ...
 *
 * nonblock(NB_ENABLE)	//enable non-blocking
 * while(!kbhit()) {
   	//do stuff
   }
 * // user input
 * nonblock(NB_DISABLE)	//return back to normal
 *
 * ...
 */

#endif  // __NONBLOCK_H
