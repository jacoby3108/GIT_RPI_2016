




//http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
//http://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html


/*if you want To create a static library

gcc -c  libterm.c -o libterm.o								compile static lib
ar  -cqv libterm.a libterm.o								Create static lib
gcc simulacion.c portsimulator.c libterm.a -o simulacion    Compile app using created library

** if you want to create a standad compile 
*   
gcc simulacion.c portsimulator.c libterm.c -o simulacion

http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
* http://www.cprogramming.com/tutorial/shared-libraries-linux-gcc.html
* http://www.adp-gmbh.ch/cpp/gcc/create_lib.html
* 
*/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(STDIN_FILENO, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche(void) 
{
  return getch_(1);
}


int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}


