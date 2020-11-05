#include "strings_and_files.h"

// Passes empty pointers of char to parse command and his parametr
// Generic command - command, which should be parsed
void parse_command(char* generic_command, char** _command, char** _parametr){
    // For logging
    //printf("Input command: %s\n", generic_command);
    int index = 0;
    int size_of_symbols = strlen(generic_command);
    while(index != size_of_symbols - 1){
        if(generic_command[index] == ' ')
            break;
        index++;
    }
    //printf("Index: %d\n", index);
    char* subbuff1 = (char*)malloc(sizeof(char) * index);
    char* subbuff2 = (char*)malloc(sizeof(char) * (size_of_symbols - index - 1));
    memcpy(subbuff1, generic_command, index);
    memcpy(subbuff2, &generic_command[index + 1], size_of_symbols - index - 1);
    // printf("Length: %d\n", size_of_symbols - index - 1);
    // printf("Length: %d\n", strlen(subbuff2));
    // printf("Type of command: %s\n", subbuff1);
    // printf("Parametr of command: %s\n", subbuff2);
    *_command = subbuff1;
    *_parametr = subbuff2;
}

// Return 0 if file not found, or 1 if file was found
int find_file_in_current_directory(char* filename){
    char* curr_dir_path_ = getcwd(NULL, 0);
    //printf("%d\n", length_of);
    DIR* dp;
    struct dirent * dirp;
    if((dp = opendir(curr_dir_path_)) == NULL){
        free(curr_dir_path_);
        return -1;
    }
    while((dirp = readdir(dp)) != NULL){
        if(!strcmp(dirp->d_name, filename)){
            free(curr_dir_path_);
            closedir(dp);
            return 1;
        }
    }
    free(curr_dir_path_);
    closedir(dp);
    return 0;
}


// Return size of read password symbols
// Input password in hide style
ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp)
{
    if (!pw || !sz || !fp) return -1;       /* validate input   */
#ifdef MAXPW
    if (sz > MAXPW) sz = MAXPW;
#endif

    if (*pw == NULL) {              /* reallocate if no address */
        void *tmp = realloc (*pw, sz * sizeof **pw);
        if (!tmp)
            return -1;
        memset (tmp, 0, sz);    /* initialize memory to 0   */
        *pw =  (char*) tmp;
    }

    size_t idx = 0;         /* index, number of chars in read   */
    int c = 0;

    struct termios old_kbd_mode;    /* orig keyboard settings   */
    struct termios new_kbd_mode;

    if (tcgetattr (0, &old_kbd_mode)) { /* save orig settings   */
        fprintf (stderr, "%s() error: tcgetattr failed.\n", __func__);
        return -1;
    }   /* copy old to new */
    memcpy (&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);  /* new kbd flags */
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr (0, TCSANOW, &new_kbd_mode)) {
        fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    /* read chars from fp, mask if valid char specified */
    while (((c = fgetc (fp)) != '\n' && c != EOF && idx < sz - 1) ||
            (idx == sz - 1 && c == 127))
    {
        if (c != 127) {
            if (31 < mask && mask < 127)    /* valid ascii char */
                fputc (mask, stdout);
            (*pw)[idx++] = c;
        }
        else if (idx > 0) {         /* handle backspace (del)   */
            if (31 < mask && mask < 127) {
                fputc (0x8, stdout);
                fputc (' ', stdout);
                fputc (0x8, stdout);
            }
            (*pw)[--idx] = 0;
        }
    }
    (*pw)[idx] = 0; /* null-terminate   */

    /* reset original keyboard  */
    if (tcsetattr (0, TCSANOW, &old_kbd_mode)) {
        fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    if (idx == sz - 1 && c != '\n') /* warn if pw truncated */
        fprintf (stderr, " (%s() warning: truncated at %zu chars.)\n",
                __func__, sz - 1);

    return idx; /* number of chars in passwd    */
}


// Read string to the delimiter End_of from the file (stream or real file)
// Warning !!! Allocated dynamic memory for string. To avoid leaking memory for char
// Pointer should be called free 
char* read_from_fp_by_symbol(FILE* fp, char End_of){
    int capacity = 1;
    int position_to_insert = 0;
    char input_symbol;
    char* my_string = (char*)malloc(sizeof(char) * capacity);
    while((input_symbol = fgetc(fp)) != End_of){
        if(position_to_insert == capacity){
            capacity *= 2;
            my_string = realloc(my_string, capacity);
        }
        my_string[position_to_insert++] = input_symbol;
        // Log
        // printf("%d - position, %d - capacity\n", position_to_insert, capacity);
    }
    // Result string
    // printf("%s\n", my_string);
    return my_string;
}
