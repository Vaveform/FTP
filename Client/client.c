#include <stdio.h>
#include <string.h>
#include "net_headers.h"


int main(int argc, int** argv){
    // char to_server_addres[11];
    // char to_server_port[6];
    // printf("Input ip address of FTP server: ");
    // fgets(to_server_addres, 11, stdin);
    // printf("Input port of remote FTP server: ");
    // fgets(to_server_port, 6, stdin);
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);


    printf("Creating socket...\n");
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Connecting...\n");
    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    char readed_data[1024];
    int bytes_recieved = recv(socket_peer, readed_data, 4096, 0);
    printf("%s\n", readed_data);
    fgets(&readed_data[strlen(readed_data) - 3], 1024 - strlen(readed_data), stdin);
    int bytes_send = send(socket_peer, readed_data, 1024, 0);
    

    return 0;
}