#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "net_headers.h"
#include <regex.h>
#include "sqlite3.h"

#define CHUNK_SIZE 128


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

struct addrinfo create_addrinfo_pattern(int _protocol_family, int _socktype, int _flags){
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = _protocol_family;
  hints.ai_socktype = _socktype;
  hints.ai_flags = _flags;
  return hints;
}

SOCKET create_listen_socket(char* address, char* port_name, int max_number_waiting_connections){
  struct addrinfo* bind_addresses;
  struct addrinfo hints = create_addrinfo_pattern(AF_INET, SOCK_STREAM, AI_PASSIVE);
  getaddrinfo(address, port_name, &hints, &bind_addresses);
  SOCKET socket_listen = socket(bind_addresses->ai_family, bind_addresses->ai_socktype, bind_addresses->ai_protocol);
  if(!ISVALIDSOCKET(socket_listen)){
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return -1;
  }
  if(bind(socket_listen, bind_addresses->ai_addr, bind_addresses->ai_addrlen)){
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return -1;
  }
  freeaddrinfo(bind_addresses);
  if(listen(socket_listen, max_number_waiting_connections) < 0){
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return -1;
  }
  return socket_listen;
}

int main(int argc, char** argv){
  if(argc < 3){
    printf("Not enough parametrs to launch\n");
    return 0;
  }
  printf("This is FTP-server\n");
  printf("Binding server to the IP-address: %s and Port: %s\n", argv[1], argv[2]);
  
  SOCKET listen_socket = create_listen_socket(argv[1], argv[2], 10);

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

  char buffer[1024];
  const char* invitation = 
    "Welcome to FTP server!!!\n"
    "To enter, please input login and password.\r\n";
  int bytes_send = send(socket_client, invitation, strlen(invitation), 0);
  int bytes_received = recv(socket_client, buffer, 1024, 0);
  printf("Got login: %s", buffer);
  bytes_received = recv(socket_client, buffer, 1024, 0);
  printf("Got password: %s\n", buffer);



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
  while((dirp = readdir(dp)) != NULL)
      printf("%s\n", dirp->d_name);
  closedir(dp);

  FILE* f = fopen("Sheme.jpg", "rb");
  size_t nbytes = 0;
  char *file_data = (char*)malloc(sizeof(char) * CHUNK_SIZE);
  while((nbytes = fread(file_data, 1, CHUNK_SIZE, f))){
    bytes_send = send(socket_client, file_data, nbytes, 0);
  }
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