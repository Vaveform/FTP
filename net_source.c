#include "net_headers.h"

char* current_time_system(){
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    return time_msg;
}

struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = _protocol_family;
    hints.ai_socktype = _socktype;
    hints.ai_flags = _flags;
    return hints;
}

// Return descriptor of open listen socket. Argument fp for logging
SOCKET create_listen_socket(char* address, char* port_name, int max_number_waiting_connections, FILE* fp){
    struct addrinfo* bind_addresses;
    struct addrinfo hints = create_addrinfo_pattern(AF_INET, SOCK_STREAM, AI_PASSIVE);
    getaddrinfo(address, port_name, &hints, &bind_addresses);
    SOCKET socket_listen = socket(bind_addresses->ai_family, bind_addresses->ai_socktype, bind_addresses->ai_protocol);
    if(!ISVALIDSOCKET(socket_listen)){
        if(fp)
            fprintf(fp, "%s: socket() failed. (%d)\n",current_time_system(), GETSOCKETERRNO());
        return -1;
    }
    if(bind(socket_listen, bind_addresses->ai_addr, bind_addresses->ai_addrlen)){
        if(fp)
            fprintf(fp, "%s: bind() failed. (%d)\n", current_time_system(), GETSOCKETERRNO());
        return -1;
    }
    freeaddrinfo(bind_addresses);
    if(listen(socket_listen, max_number_waiting_connections) < 0){
        if(fp)
            fprintf(fp, "%s: listen() failed. (%d)\n", current_time_system(), GETSOCKETERRNO());
        return -1;
    }
    return socket_listen;
}

// Connect to listen server and return socket of connection(client)
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
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return -1;
    }

    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return -1;
    }
    freeaddrinfo(peer_address);

    return socket_peer;
}

// Write data to open file with 'ab' flags from received data from socket_peer
// This function don't close FILE
size_t recv_file_from_peer(SOCKET socket_peer, FILE* to_write, int _chunck_size){
    char* reicived_data = (char*)malloc(sizeof(char) * _chunck_size);
    size_t summary_bytes_recieved = 0;
    size_t bytes_recieved = 0;
    while(1){
        bytes_recieved = recv(socket_peer, reicived_data, _chunck_size, 0);
        summary_bytes_recieved += bytes_recieved;
        if(bytes_recieved == 0){
            break;
        }
        fwrite(reicived_data, 1, bytes_recieved, to_write);
    }
    free(reicived_data);
    return summary_bytes_recieved;
}

// Read data from open file with 'rb' flags and send to socket_peer
// This function don't close FILE
size_t send_file_to_peer(SOCKET socket_peer, FILE* to_read, int _chunck_size){
    size_t nbytes = 0;
    size_t bytes_send = 0;
    size_t summary_bytes_sent = 0;
    char *file_data = (char*)malloc(sizeof(char) * _chunck_size);
    while((nbytes = fread(file_data, 1, _chunck_size, to_read))){
        bytes_send = send(socket_peer, file_data, nbytes, 0);
        summary_bytes_sent += bytes_send;
    }
    free(file_data);
    return summary_bytes_sent;
}