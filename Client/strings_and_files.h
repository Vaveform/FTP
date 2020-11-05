#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>


// Passes empty pointers of char to parse command and his parametr
// Generic command - command, which should be parsed
void parse_command(char* generic_command, char** _command, char** _parametr);



// Return 0 if file not found, or 1 if file was found
int find_file_in_current_directory(char* filename);



// Return size of read password symbols
// Input password in hide style
ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp);


// Read string to the delimiter End_of from the file (stream or real file)
// Warning !!! Allocated dynamic memory for string. To avoid leaking memory for char
// Pointer should be called free 
char* read_from_fp_by_symbol(FILE* fp, char End_of);