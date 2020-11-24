// It is main headers for working with sockets

#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "strings_and_files.h"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#define CHUNK_SIZE 128



// Output tokens after user authorization
typedef enum Authorization_Token{
    USER_NOT_FOUND_IN_SERVER_DATABASE = 404,
    USER_FOUND_IN_SERVER_DATABASE = 200,
    USER_NEW_ON_SERVER = 101,
} Authorization_Token;

typedef enum Registration_Token{
    REGISTRATION_FAILED = 560,
    REGISTRATION_COMPLETE = 310,
    REGISTRATION_NOT_BEGIN = 15,
} Registration_Token;

typedef enum Client_Status{
    DISCONNECTING = 0x00AF,
    IN_AUTHORIZATION = 0x00BF,
    IN_REGISTRATION = 0x00CF,
    AUTHORIZATED = 0x00DF,
} Client_Status;

typedef struct Client_FTP_Message{
    Authorization_Token auth_token;
    Registration_Token reg_token;
    Client_Status status;
    char message[COMMAND_SIZE];
} Client_FTP_Message;

char* current_time_system();

struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags);

// Return descriptor of open listen socket. Argument fp for logging
SOCKET create_listen_socket(char* address, char* port_name, int max_number_waiting_connections, FILE* fp);

// Connect to listen server and return socket of connection(client)
// Listen server should be bind to passing ip_address and port
// And started listen with listen()
SOCKET connect_to_listen_server(char* ip_address, char* port);

// Write data to open file with 'ab' flags from received data from socket_peer
// This function don't close FILE
size_t recv_file_from_peer(SOCKET socket_peer, FILE* to_write, int _chunck_size);

// Read data from open file with 'rb' flags and send to socket_peer
// This function don't close FILE
size_t send_file_to_peer(SOCKET socket_peer, FILE* to_read, int _chunck_size);









