#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_headers.h"
#include "strings_and_files.h"


// // // Write data to open file with 'ab' flags from received data from socket_peer
// // // This function don't close FILE
// // size_t recv_file_from_peer(SOCKET socket_peer, FILE* to_write, int _chunck_size){
// //     char* reicived_data = (char*)malloc(sizeof(char) * _chunck_size);
// //     size_t summary_bytes_recieved = 0;
// //     size_t bytes_recieved = 0;
// //     while(1){
// //         bytes_recieved = recv(socket_peer, reicived_data, _chunck_size, 0);
// //         summary_bytes_recieved += bytes_recieved;
// //         if(bytes_recieved == 0){
// //             break;
// //         }
// //         fwrite(reicived_data, 1, bytes_recieved, to_write);
// //     }
// //     free(reicived_data);
// //     return summary_bytes_recieved;
// // }

// // // Read data from open file with 'rb' flags and send to socket_peer
// // // This function don't close FILE
// // size_t send_file_to_peer(SOCKET socket_peer, FILE* to_read, int _chunck_size){
// //     size_t nbytes = 0;
// //     size_t bytes_send = 0;
// //     size_t summary_bytes_sent = 0;
// //     char *file_data = (char*)malloc(sizeof(char) * CHUNK_SIZE);
// //     while((nbytes = fread(file_data, 1, CHUNK_SIZE, to_read))){
// //         bytes_send = send(socket_peer, file_data, nbytes, 0);
// //         summary_bytes_sent += bytes_send;
// //     }
// //     free(file_data);
// //     return summary_bytes_sent;
// // }

// // Passes empty pointers of char to parse command and his parametr
// // Generic command - command, which should be parsed
// void parse_command(char* generic_command, char** _command, char** _parametr){
//     // For logging
//     //printf("Input command: %s\n", generic_command);
//     int index = 0;
//     int size_of_symbols = strlen(generic_command);
//     while(index != size_of_symbols - 1){
//         if(generic_command[index] == ' ')
//             break;
//         index++;
//     }
//     //printf("Index: %d\n", index);
//     char* subbuff1 = (char*)malloc(sizeof(char) * index);
//     char* subbuff2 = (char*)malloc(sizeof(char) * (size_of_symbols - index - 1));
//     memcpy(subbuff1, generic_command, index);
//     memcpy(subbuff2, &generic_command[index + 1], size_of_symbols - index - 1);
//     // printf("Length: %d\n", size_of_symbols - index - 1);
//     // printf("Length: %d\n", strlen(subbuff2));
//     // printf("Type of command: %s\n", subbuff1);
//     // printf("Parametr of command: %s\n", subbuff2);
//     *_command = subbuff1;
//     *_parametr = subbuff2;
// }

// // Return 0 if file not found, or 1 if file was found
// int find_file_in_current_directory(char* filename){
//     char* curr_dir_path_ = getcwd(NULL, 0);
//     //printf("%d\n", length_of);
//     DIR* dp;
//     struct dirent * dirp;
//     if((dp = opendir(curr_dir_path_)) == NULL){
//         free(curr_dir_path_);
//         return -1;
//     }
//     while((dirp = readdir(dp)) != NULL){
//         if(!strcmp(dirp->d_name, filename)){
//             free(curr_dir_path_);
//             closedir(dp);
//             return 1;
//         }
//     }
//     free(curr_dir_path_);
//     closedir(dp);
//     return 0;
// }


// // Return size of read password symbols
// // Input password in hide style
// ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp)
// {
//     if (!pw || !sz || !fp) return -1;       /* validate input   */
// #ifdef MAXPW
//     if (sz > MAXPW) sz = MAXPW;
// #endif

//     if (*pw == NULL) {              /* reallocate if no address */
//         void *tmp = realloc (*pw, sz * sizeof **pw);
//         if (!tmp)
//             return -1;
//         memset (tmp, 0, sz);    /* initialize memory to 0   */
//         *pw =  (char*) tmp;
//     }

//     size_t idx = 0;         /* index, number of chars in read   */
//     int c = 0;

//     struct termios old_kbd_mode;    /* orig keyboard settings   */
//     struct termios new_kbd_mode;

//     if (tcgetattr (0, &old_kbd_mode)) { /* save orig settings   */
//         fprintf (stderr, "%s() error: tcgetattr failed.\n", __func__);
//         return -1;
//     }   /* copy old to new */
//     memcpy (&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

//     new_kbd_mode.c_lflag &= ~(ICANON | ECHO);  /* new kbd flags */
//     new_kbd_mode.c_cc[VTIME] = 0;
//     new_kbd_mode.c_cc[VMIN] = 1;
//     if (tcsetattr (0, TCSANOW, &new_kbd_mode)) {
//         fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
//         return -1;
//     }

//     /* read chars from fp, mask if valid char specified */
//     while (((c = fgetc (fp)) != '\n' && c != EOF && idx < sz - 1) ||
//             (idx == sz - 1 && c == 127))
//     {
//         if (c != 127) {
//             if (31 < mask && mask < 127)    /* valid ascii char */
//                 fputc (mask, stdout);
//             (*pw)[idx++] = c;
//         }
//         else if (idx > 0) {         /* handle backspace (del)   */
//             if (31 < mask && mask < 127) {
//                 fputc (0x8, stdout);
//                 fputc (' ', stdout);
//                 fputc (0x8, stdout);
//             }
//             (*pw)[--idx] = 0;
//         }
//     }
//     (*pw)[idx] = 0; /* null-terminate   */

//     /* reset original keyboard  */
//     if (tcsetattr (0, TCSANOW, &old_kbd_mode)) {
//         fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
//         return -1;
//     }

//     if (idx == sz - 1 && c != '\n') /* warn if pw truncated */
//         fprintf (stderr, " (%s() warning: truncated at %zu chars.)\n",
//                 __func__, sz - 1);

//     return idx; /* number of chars in passwd    */
// }


// // Read string to the delimiter End_of from the file (stream or real file)
// // Warning !!! Allocated dynamic memory for string. To avoid leaking memory for char
// // Pointer should be called free 
// char* read_from_fp_by_symbol(FILE* fp, char End_of){
//     int capacity = 1;
//     int position_to_insert = 0;
//     char input_symbol;
//     char* my_string = (char*)malloc(sizeof(char) * capacity);
//     while((input_symbol = fgetc(fp)) != End_of){
//         if(position_to_insert == capacity){
//             capacity *= 2;
//             my_string = realloc(my_string, capacity);
//         }
//         my_string[position_to_insert++] = input_symbol;
//         // Log
//         // printf("%d - position, %d - capacity\n", position_to_insert, capacity);
//     }
//     // Result string
//     // printf("%s\n", my_string);
//     return my_string;
// }


// // struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags){
// //   struct addrinfo hints;
// //   memset(&hints, 0, sizeof(hints));
// //   hints.ai_family = _protocol_family;
// //   hints.ai_socktype = _socktype;
// //   hints.ai_flags = _flags;
// //   return hints;
// // }


// // // Connect to listen server and return socket of connection
// // // Listen server should be bind to passing ip_address and port
// // // And started listen with listen()
// // SOCKET connect_to_listen_server(char* ip_address, char* port){
// //     printf("Configuring remote address...\n");
// //     struct addrinfo hints = create_addrinfo_pattern(NULL, SOCK_STREAM, NULL);
// //     struct addrinfo* peer_address;
// //     if(getaddrinfo(ip_address, port, &hints, &peer_address)){
// //         fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
// //         return -1;
// //     }

// //     char address[100];
// //     char service[100];
// //     getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address, sizeof(address), service, sizeof(service), NI_NUMERICHOST);
// //     printf("Remote address is: %s %s\n", address, service);

// //     SOCKET socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
// //     if(!ISVALIDSOCKET(socket_peer)){
// //         fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
// //         return -1;
// //     }

// //     if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
// //         fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
// //         return -1;
// //     }
// //     freeaddrinfo(peer_address);

// //     return socket_peer;
// // }

int main(int argc, int** argv){

    // Socket for chatting with server
    SOCKET socket_peer = connect_to_listen_server(argv[1], "21");
    if(!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "invalid value of SOCKET for command");
        return -1;
    }
    // Socket for sending and receiving data
    SOCKET socket_peer_data = connect_to_listen_server(argv[1], "20");
    if(!ISVALIDSOCKET(socket_peer_data)){
        fprintf(stderr, "invalid value of SOCKET for data");
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
    // Creating pointer of buffer 'readed data' for function getpasswrd
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
    // Code near emulate active communication mode 
    char generic_command[256];
    int flag_operation;
    fgets(generic_command, 256, stdin);
    char* command, *parametr;
    parse_command(generic_command, &command, &parametr);
    if(!strcmp(command, "get")){
        // Check if the file with passed name exist
        if(find_file_in_current_directory(parametr)){
            printf("File already exist!!!\n");
        }
        else{
            // We should send to the ftp server name of the file
            // Server found/not found the file and send him to the client by binded socket 21
            send(socket_peer, generic_command, 256, 0);
            // Response from server - int number
            recv(socket_peer, &flag_operation, sizeof(int), 0);
            // If flag_operation not zero
            if(flag_operation){
                // From server socket on port 20 sending data
                // Here should be error handling
                FILE* to_write = fopen(parametr, "ab");
                // Recieving file data from server
                printf("Recieved file %s with size %lu bytes\n", parametr, recv_file_from_peer(socket_peer_data, to_write, 128));
                fclose(to_write);
            }
            else
            {
                printf("File not exist on the FTP server\n");
            }
        }
        

    }
    else if(!strcmp(command, "put")){
        if(!find_file_in_current_directory(parametr)){
            printf("File not exists!!!\n");
        }
        else{
            // Sending command to the server
            send(socket_peer, generic_command, 256, 0);
            // Response from server - int number - server ready to recv file
            recv(socket_peer, &flag_operation, sizeof(int), 0);
            if(flag_operation){
                // Open file to send data
                // Here should be error handling
                FILE* to_read = fopen(parametr, "rb");
                // Sending to the server
                printf("Sent file %s with size %lu bytes\n", parametr, send_file_to_peer(socket_peer_data, to_read, 128));
                fclose(to_read);
            }
            else{
                printf("File already exist on the FTP server\n");
            }
        }
    }
    else if(!strcmp(command, "ls")){
        // List of the file on the server
        send(socket_peer, generic_command, 256, 0);
        recv(socket_peer, readed_data, 1024, 0);
        printf("%s\n", readed_data);

    }
    else if(!strcmp(command, "exit")){
        printf("Finished...\n");
        free(command);
        free(parametr);
        CLOSESOCKET(socket_peer);
        CLOSESOCKET(socket_peer_data);
        return 0;

    }
    else
    {
        printf("Invalid command\n");
    }
    //Close connection with server
    free(command);
    free(parametr);
    CLOSESOCKET(socket_peer);
    CLOSESOCKET(socket_peer_data);

    return 0;
}