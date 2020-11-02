#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_headers.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>


#define SENDING_BUFFER_SIZE 128


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

// Output - 1 - file exist/ 0 file not exist
int find_file_in_current_directory(char* filename){
    char buf[1024];
    char* simple;
    simple = getcwd(buf, 1024);
    size_t length_of = strlen(simple);
    printf("%d\n", length_of);
    char buffer[length_of];
    strcpy(buffer, buf);
    DIR* dp;
    struct dirent * dirp;
    if((dp = opendir(buffer)) == NULL){
        return -1;
    }
    while((dirp = readdir(dp)) != NULL){
        if(!strcmp(dirp->d_name, filename)){
            closedir(dp);
            return 1;
        }
    }
    closedir(dp);
    return 0;
}

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


struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags){
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = _protocol_family;
  hints.ai_socktype = _socktype;
  hints.ai_flags = _flags;
  return hints;
}


// Connect to listen server and return socket of connection
// Listen server should be bind to passing ip_address and port
// And started listen with listen()
SOCKET connect_to_listen_server(char* ip_address, char* port){
    printf("Configuring remote address...\n");
    struct addrinfo hints = create_addrinfo_pattern(NULL, SOCK_STREAM, NULL);
    struct addrinfo* peer_address;
    if(getaddrinfo(ip_address, port, &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return -1;
    }

    char address[100];
    char service[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address, sizeof(address), service, sizeof(service), NI_NUMERICHOST);
    printf("Remote address is: %s %s\n", address, service);

    SOCKET socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_peer)){
        printf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return -1;
    }

    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return -1;
    }
    freeaddrinfo(peer_address);

    return socket_peer;
}

int main(int argc, int** argv){

    // // FILE* fp = fopen("file.txt", "r");
    // // read_from_fp_by_symbol(fp, '\n');
    // FILE* fp1 = fopen("file2.txt", "ab");
    // // fputs("test file hello", fp1);
    // char* test_string = "sdfsdfsdfsfsdfsdfsdf";
    // fwrite(test_string, 1, 21, fp1);
    // fclose(fp1);
    // // fclose(fp);

    SOCKET socket_peer = connect_to_listen_server(argv[1], "21");
    if(!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "invalid SOCKET value");
        return -1;
    }

    char readed_data[1024];
    // Receive invitation and print them
    int bytes_recieved = recv(socket_peer, readed_data, 1024, 0);
    printf("%s", readed_data);
    // Invitation to input login:
    printf("Login: ");
    fgets(readed_data, 1024, stdin);
    // Sending input login
    int bytes_send = send(socket_peer, readed_data, 1024, 0);
    // Invitation to input password
    printf("Password: ");
    // Createing pointer of buffer 'readed data' for function getpasswrd
    char* pointer_to_password = readed_data;
    // Call function getpasswd - inputing password with hide symbols
    getpasswd(&pointer_to_password, 1024, '*', stdin);
    // Sending password
    bytes_send = send(socket_peer, readed_data, 1024, 0);
    printf("\n");
    

    // command line interface:
    // put filename - sending file to ftp server
    // get filename - load file from server directory
    // ls - list of the files in the server directory 
    char command[256];
    fgets(command, 256, stdin);
    if(c) 
    

    // get implementation
    if(remove("Sheme.jpg"))
        printf("File named \"Sheme.jpg\" already exists. Deleted...");
    else
        printf("File named \"Sheme.jpg\" doesn't exist. Saving file");
    // Creating file (if this file not exists) with special modes: a - additional recording 
    // with b - binary to recv any file 
    FILE* to_write = fopen("Sheme.jpg", "ab");
    // Recieving file data from server
    
    char* reicived_data = (char*)malloc(sizeof(char) * SENDING_BUFFER_SIZE);
    while(1){
        bytes_recieved = recv(socket_peer, reicived_data, SENDING_BUFFER_SIZE, 0);
        if(bytes_recieved == 0){
            break;
        }
        fwrite(reicived_data, 1, bytes_recieved, to_write);
    }
    fclose(to_write);
    free(reicived_data);
    
    //Close connection with server
    CLOSESOCKET(socket_peer);
    return 0;
}