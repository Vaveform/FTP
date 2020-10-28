#include <stdio.h>
#include <string.h>
#include "net_headers.h"
#include "sqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


// This function create listen_socket for address and port_name and return them.
// After calling of this function need to call bind to tie with local address using
// First element of passed "bind_addresses", which after calling will be filled 
SOCKET initialize_server_socket(char* address, char* port_name, struct addrinfo* bind_addresses){
    printf("This is FTP-server\n");
    printf("Binding server to the IP-address: %s and Port: %s\n", address, port_name);
    // hints - addrinfo structure. This is structure point to preferable type of
    // socket or protocol 
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    // Filling structure hints - what will be find
    // addrinfo.ai_family - looking for an IPv4 addresses (AF_INET6 - IPv6 addresses)
    hints.ai_family = AF_INET;
    // addrinfo.ai_socktype - type of transport protocol: SOCK_DGRAM - UDP or SOCK_STREAM - TCP
    hints.ai_socktype = SOCK_STREAM;
    // addrinfo.ai_flags - to recieve connections from clients, which have any net address
    hints.ai_flags = AI_PASSIVE;
    // Fill passed link list of addrinfo structure. By one for every net address subject to
    // Seleected hints structure pararmetrs  
    getaddrinfo(0, "8080", &hints, &bind_addresses);
    printf("Creating socket...\n");
    // int socket(int domain, int type, int protocol);
    // domain - this parametr select set of the protocols, which will be using for creation connection
    // type - this parametr select semantics of communication (tranport protocols for example TCP/UDP)
    // protocol - this parametr set concrete type of protocol
    return socket(bind_addresses->ai_family, bind_addresses->ai_socktype, bind_addresses->ai_protocol);
}

int main(int argc, char** argv){
  if(argc < 3){
    printf("Not enough parametrs to launch\n");
    return 0;
  }
  printf("This is FTP-server\n");
  printf("Binding server to the IP-address: %s and Port: %s\n", argv[1], argv[2]);
  // hints - addrinfo structure. This is structure point to preferable type of
  // socket or protocol 
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  // Filling structure hints - what will be find
  // addrinfo.ai_family - looking for an IPv4 addresses (AF_INET6 - IPv6 addresses)
  hints.ai_family = AF_INET;
  // addrinfo.ai_socktype - type of transport protocol: SOCK_DGRAM - UDP or SOCK_STREAM - TCP
  hints.ai_socktype = SOCK_STREAM;
  // addrinfo.ai_flags - to recieve connections from clients, which have any net address
  hints.ai_flags = AI_PASSIVE;
  // Fill passed link list of addrinfo structure. By one for every net address subject to
  // Seleected hints structure pararmetrs 
  struct addrinfo* bind_address; 
  getaddrinfo(argv[1], argv[2], &hints, &bind_address);
  printf("Creating socket...\n");
  // int socket(int domain, int type, int protocol);
  // domain - this parametr select set of the protocols, which will be using for creation connection
  // type - this parametr select semantics of communication (tranport protocols for example TCP/UDP)
  // protocol - this parametr set concrete type of protocol
  // SOCKET listen_socket = initialize_server_socket(argv[1], argv[2], bind_address);
  SOCKET listen_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
  // To check on valid or not valid returned socket 
  if(!ISVALIDSOCKET(listen_socket)){
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  // Binding returned socket with local address with using filled bind_address
  printf("Binding...\n");
  // printf("%d\n", (int)bind_address->ai_addrlen);
  if(bind(listen_socket, bind_address->ai_addr, bind_address->ai_addrlen)){
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  // Free resources (address storage) related with bind_address
  freeaddrinfo(bind_address);
  // Start listening on listen_socket. Second parametr set the max number of input
  // connections to the queue. If queue overflowed all input connection will be rejected
  printf("Listening connections...\n");
  if(listen(listen_socket, 10) < 0){
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    CLOSESOCKET(listen_socket);
    return 1;
  }

  printf("Waiting a connection...\n");
  // client_address - to store address info for the connecting client
  struct sockaddr_storage client_address;
  // To call accept and getnameinfo it is neccessary to pass size of client_address
  socklen_t client_len = sizeof(client_address);


  // Programm blocked until new input connection
  SOCKET socket_client = accept(listen_socket, (struct sockaddr*)&client_address, &client_len);

  if(!ISVALIDSOCKET(socket_client)){
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    CLOSESOCKET(listen_socket);
    return 1;
  }

  printf("Client is connected...\n");
  char address_buffer[100];
  getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  const char* invitation = 
    "Welcome to FTP server!!!\n"
    "To enter, please input login and password.\n";
  char enter_login[1024] = "Login:";
  int bytes_send = send(socket_client, invitation, strlen(invitation), 0);
  printf("Sent %d of %d bytes.\n", bytes_send, strlen(invitation));
  bytes_send = send(socket_client, enter_login, strlen(enter_login), 0);
  printf("Sent %d of %d bytes.\n", bytes_send, strlen(invitation));

  int bytes_received = recv(socket_client, enter_login, 1024, 0);
  printf("Receive %d bytes\n", bytes_received);
  printf("Input %s\n", enter_login);
  CLOSESOCKET(listen_socket);
  printf("Finished\n");


    // sqlite3 *db;
    // char *zErrMsg = 0;
    // int rc;
    // if( argc!=3 ){
    //   fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
    //   return(1);
    // }
    // rc = sqlite3_open(argv[1], &db);
    // if( rc ){
    //   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //   sqlite3_close(db);
    //   return 1;
    // }
    // const char request[] = "CREATE TABLE contacts (column_1 data_type PRIMARY KEY, column_2 data_type NOT NULL, column_3 data_type DEFAULT 0,table_constraints);";
    // rc = sqlite3_exec(db, &request, callback, 0, &zErrMsg);
    // if( rc!=SQLITE_OK ){
    //   fprintf(stderr, "SQL error: %s\n", zErrMsg);
    //   sqlite3_free(zErrMsg);
    // }
    // sqlite3_close(db);

    return 0;
}