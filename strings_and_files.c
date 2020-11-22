#include "strings_and_files.h"

// Passes empty pointers of char to parse command and his parametr
// Generic command - command, which should be parsed
void parse_command(char* generic_command, char** _command, char** _parametr){
    //For logging
    // printf("Input command: %s and her length: %lu\n", generic_command, strlen(generic_command));
    size_t index = 0;
    size_t symbols_length = strlen(generic_command);
    if(symbols_length <= 0)
        return ;
    for(;index < symbols_length; index++){
        if(generic_command[index] == ' ' || generic_command[index] == '\n'){
            break;
        }
    }
    
    *_command = (char*)calloc(index, sizeof(char));
    memcpy(*_command, generic_command, index);
    // printf("Command: %s and length: %lu\n", *_command, strlen(*_command));
    
    int parametr_size = strlen(generic_command) - index - 1;

    //printf("Parametr size: %lu and index: %d\n", parametr_size, index);
    
    if(parametr_size > 0){
        
        *_parametr = (char*)calloc((size_t)parametr_size, sizeof(char));
        memcpy(*_parametr, &generic_command[index + 1],(size_t)parametr_size);
        // printf("Argument: %s and length: %lu\n", *_parametr, strlen(*_parametr));
    }
    else
    {
        *_parametr = NULL;
    }
    //printf("Returned command: %s and returned parametr: %s\n", *_command, *_parametr);
}


// Return 0 if file not found, or 1 if file was found
int find_file_in_current_directory(char* filename){
    char* curr_dir_path_ = getcwd(NULL, 0);
    // printf("%s\n", filename);
    DIR* dp;
    struct dirent * dirp;
    if((dp = opendir(curr_dir_path_)) == NULL){
        free(curr_dir_path_);
        return -1;
    }
    
    while((dirp = readdir(dp)) != NULL){
        // printf("%s : %s - %d : %d\n", dirp->d_name, filename, strlen(dirp->d_name), strlen(filename));
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
    size_t capacity = 1;
    size_t position_to_insert = 0;
    char input_symbol;
    char* _string = (char*)malloc(sizeof(char) * capacity);
    while((input_symbol = fgetc(fp)) != End_of){
        if(position_to_insert == capacity){
            capacity *= 2;
            _string = realloc(_string, capacity);
        }
        _string[position_to_insert++] = input_symbol;
        // Log
        // printf("%d - position, %d - capacity\n", position_to_insert, capacity);
    }
    // Result string
    // printf("%s\n", my_string);
    _string = realloc(_string, position_to_insert);
    return _string;
}


// Return dynamic allocated string with list of all files in directory
char* files_in_current_directory(){
    char* curr_dir_path_ = getcwd(NULL, 0);
    DIR* dp;
    struct dirent * dirp;
    if((dp = opendir(curr_dir_path_)) == NULL){
        free(curr_dir_path_);
        return NULL;
    }
    size_t bytes_ = 0;
    size_t index_to_copy = 0;
    size_t file_name_len = 0;
    char* _files = (char*)calloc(sizeof(char), 1);
    while((dirp = readdir(dp)) != NULL){
        if(strcmp("..", dirp->d_name) && strcmp(".", dirp->d_name)){
            file_name_len = strlen(dirp->d_name);
            bytes_ += (file_name_len * sizeof(char) + 1);
            _files = realloc(_files, bytes_);
            memcpy(&_files[index_to_copy], dirp->d_name, file_name_len);
            _files[bytes_ - 1] = '\n';
            index_to_copy = bytes_;
        }
    }
    free(curr_dir_path_);
    closedir(dp);
    return _files;
}