#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#ifdef _WIN32
    #include <conio.h> 
#else 
    #include <termios.h> 
    #include <unistd.h> 
    #include <fcntl.h>  
#endif


void setNonBlockingInput();


void restoreBlockingInput();

char getNonBlockingChar();

#endif 