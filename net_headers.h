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

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#define CHUNK_SIZE 128

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









