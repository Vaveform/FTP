#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../net_headers.h"

#include "../strings_and_files.h"

int main(int argc, int** argv){

    // Socket for chatting with server
    SOCKET socket_peer = connect_to_listen_server(argv[1], "21");
    if(!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "invalid value of SOCKET for command\n");
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
    char* command, *parametr;
    int flag_operation;
    while(1){
        fgets(generic_command, 256, stdin);
        printf("Printed command: %s\n", generic_command);
        parse_command(generic_command, &command, &parametr);
        //printf("command: %s command_strlen: %lu; parametr: %s parametr_strlen: %lu\n", command,strlen(command), parametr, strlen(parametr));
        
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


                    // Socket for sending and receiving data
                    SOCKET socket_peer_data = connect_to_listen_server(argv[1], "20");
                    if(!ISVALIDSOCKET(socket_peer_data)){
                        fprintf(stderr, "invalid value of SOCKET for data\n");
                        return -1;
                    }
                    // Recieving file data from server
                    printf("Recieved file %s with size %lu bytes\n", parametr, recv_file_from_peer(socket_peer_data, to_write, 128));
                    fclose(to_write);
                    CLOSESOCKET(socket_peer_data);
                }
                else
                {
                    printf("File not exist on the FTP server\n");
                }
                // printf("%s with %s\n", command, parametr);
            }
            

        }
        else if(!strcmp(command, "put")){
            if(!find_file_in_current_directory(parametr)){
                printf("File not exists!!!\n");
            }
            else{
                // printf("%s with %s\n", command, parametr);
                // Sending command to the server
                send(socket_peer, generic_command, 256, 0);
                // Response from server - int number - server ready to recv file
                recv(socket_peer, &flag_operation, sizeof(int), 0);
                if(flag_operation){
                    // Open file to send data
                    // Here should be error handling
                    FILE* to_read = fopen(parametr, "rb");


                    // Socket for sending and receiving data
                    SOCKET socket_peer_data = connect_to_listen_server(argv[1], "20");
                    if(!ISVALIDSOCKET(socket_peer_data)){
                        fprintf(stderr, "invalid value of SOCKET for data\n");
                        return -1;
                    }

                    // Sending to the server
                    printf("Sent file %s with size %lu bytes\n", parametr, send_file_to_peer(socket_peer_data, to_read, 128));
                    fclose(to_read);
                    CLOSESOCKET(socket_peer_data);
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
            printf("%s\r\n", readed_data);
            // printf("%s\n", command);

        }
        else if(!strcmp(command, "exit")){
            printf("Finished...\n");
            send(socket_peer, generic_command, 256, 0);
            free(command);
            free(parametr);
            break;

        }
        else
        {
            send(socket_peer, generic_command, 256, 0);
            printf("Invalid command\n");
        }
        
        free(command);
        free(parametr);
    }
    //Close connection with server
    CLOSESOCKET(socket_peer);
    //CLOSESOCKET(socket_peer_data);



    return 0;
}