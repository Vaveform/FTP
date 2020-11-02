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

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)


#define CHUNK_SIZE 128


struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags);

SOCKET create_listen_socket(char* address, char* port_name, int max_number_waiting_connections);

// Connect to listen server and return socket of connection(client)
// Listen server should be bind to passing ip_address and port
// And started listen with listen()
SOCKET connect_to_listen_server(char* ip_address, char* port);

// Receive file data. socket_peer_20 - which for data
// And socket_peer_21 - to send name of file and get response of server (file exist or not)
// Number 20 and 21 in name of pararmetrs - number of ports 
size_t load_from_ftp(SOCKET socket_peer_20, SOCKET socket_peer_21, char* filename);

// Send file data. socket_peer_20 - which for data
// And socket_peer_21 - to send name of file and get response of server
// Number 20 and 21 in name of pararmetrs - number of ports 
size_t send_to_ftp(SOCKET socket_peer_20, SOCKET socket_peer_21, char* filename);









