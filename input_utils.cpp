#include "input_utils.h"
#include <iostream> // For error messages, if any

#ifdef _WIN32
    // No specific global state management needed for _kbhit/_getch beyond standard usage
#else // Linux, macOS, etc.
    static struct termios old_termios; // Static to maintain state across calls
    static bool original_termios_saved = false; // Flag to ensure we only save once
#endif

void setNonBlockingInput() {
#ifdef _WIN32
    // Windows conio.h functions are generally non-blocking by default
    // or handle internal state for simple non-blocking char reads.
    // No explicit setup needed here for basic _kbhit/_getch.
#else // Linux, macOS, etc.
    if (!original_termios_saved) {
        if (tcgetattr(STDIN_FILENO, &old_termios) == -1) {
            std::cerr << "Error: Could not get terminal attributes." << std::endl;
            return; // Or throw an exception
        }
        original_termios_saved = true;
    }

    struct termios new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode (line buffering) and echo
    new_termios.c_cc[VMIN] = 0; // Read 0 characters minimum
    new_termios.c_cc[VTIME] = 0; // No timeout for read

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        std::cerr << "Error: Could not set terminal attributes." << std::endl;
        // Attempt to restore if possible, or exit
    }

    // Make stdin non-blocking using fcntl
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Error: Could not get file flags." << std::endl;
        return;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Error: Could not set O_NONBLOCK." << std::endl;
        // Attempt to restore if possible
    }
#endif
}

void restoreBlockingInput() {
#ifdef _WIN32
    // No explicit restoration needed for basic _kbhit/_getch usage.
#else // Linux, macOS, etc.
    if (original_termios_saved) {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &old_termios) == -1) {
            std::cerr << "Error: Could not restore terminal attributes." << std::endl;
        }
        // Restore blocking for stdin
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags != -1) {
            if (fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK) == -1) {
                std::cerr << "Error: Could not restore blocking." << std::endl;
            }
        }
    }
#endif
}

char getNonBlockingChar() {
#ifdef _WIN32
    if (_kbhit()) {
        return _getch();
    }
    return 0; // Return 0 if no key is pressed
#else // Linux, macOS, etc.
    char buffer;
    ssize_t bytes_read = read(STDIN_FILENO, &buffer, 1);
    if (bytes_read > 0) {
        return buffer;
    }
    return 0; // Return 0 if no key is pressed
#endif
}

//end of input_utils.cpp

