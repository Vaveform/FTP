#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "../net_headers.h"
#include "../strings_and_files.h"


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main(int argc, char** argv){
  // if(argc < 3){
  //   printf("Not enough parametrs to launch\n");
  //   return 0;
  // }
  printf("This is FTP-server\n");
  printf("Binding server to the IP-address: %s and Port: 21\n", argv[1]);
  
  SOCKET listen_socket = create_listen_socket(argv[1], "21", 10);
  if(!ISVALIDSOCKET(listen_socket)){
    fprintf(stderr, "Invalid listen_socket value. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  SOCKET listen_socket_data = create_listen_socket(argv[1], "20", 10);
  if(!ISVALIDSOCKET(listen_socket_data)){
    fprintf(stderr, "Invalid listen_socket_data value. (%d)\n", GETSOCKETERRNO());
  }


  printf("Waiting a connection...\n");
  // client_address - to store address info for the connecting client
  struct sockaddr_storage client_address;
  // To call accept and getnameinfo it is neccessary to pass size of client_address
  socklen_t client_len = sizeof(client_address);


  // Programm blocked until new input connection
  SOCKET socket_client = accept(listen_socket, (struct sockaddr*)&client_address, &client_len);

  puts("Connected!!!\n");
  

  if(!ISVALIDSOCKET(socket_client)){
    fprintf(stderr, "accept() for listen_socket failed. (%d)\n", GETSOCKETERRNO());
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
  char generic_command[256];
  char* command, *parametr;
  while(1){
    printf("Circle...\n");
    bytes_received = recv(socket_client, generic_command, 256, 0);
    printf("Received generic command: %s\n", generic_command);
    parse_command(generic_command, &command, &parametr);
    //printf("Command: %s param: %s; param strlen: %d\n", command, parametr, strlen(parametr));
    int flag_operation = 1;
    if(!strcmp(command,"get")){
      if(!find_file_in_current_directory(parametr)){
        flag_operation = 0;
        send(socket_client, &flag_operation, sizeof(int), 0);
        printf("File not found!!!\n");
      }
      else{
        flag_operation = 1;
        send(socket_client, &flag_operation, sizeof(int), 0);
        // Her should be error handling
        FILE* to_read = fopen(parametr, "rb");

        SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
        if(!ISVALIDSOCKET(socket_client_data)){
          fprintf(stderr, "accept() for listen_socket_data failed. (%d)\n", GETSOCKETERRNO());
          CLOSESOCKET(listen_socket_data);
          return 1;
        }
        printf("Sent file with name %s to client with size: %lu bytes\n", parametr, send_file_to_peer(socket_client_data, to_read, 128));
        fclose(to_read);
        CLOSESOCKET(socket_client_data);
      }
    }
    else if(!strcmp(command, "put")){
      if(find_file_in_current_directory(parametr)){
        flag_operation = 0;
        send(socket_client, &flag_operation, sizeof(int), 0);
        printf("File already exist!!!\n");
      }
      else{
        flag_operation = 1;
        send(socket_client, &flag_operation, sizeof(int), 0);
        FILE* to_write = fopen(parametr, "ab");

        SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
        if(!ISVALIDSOCKET(socket_client_data)){
          fprintf(stderr, "accept() for listen_socket_data failed. (%d)\n", GETSOCKETERRNO());
          CLOSESOCKET(listen_socket_data);
          return 1;
        }
        printf("Received file with name %s from client with size: %lu bytes\n", parametr, recv_file_from_peer(socket_client_data, to_write, 128));
        fclose(to_write);
        CLOSESOCKET(socket_client_data);
      }
    }
    else if(!strcmp(command, "ls")){
      char* list_of_files = files_in_current_directory();
      send(socket_client, list_of_files, 1024, 0);
      free(list_of_files);
    }
    else if(!strcmp(command, "exit")){
      printf("Finished\n");
      free(command);
      free(parametr);
      break;
    }
    else{
      printf("Command not found!!!\n");
      free(command);
      free(parametr);
      break;
    }
    free(command);
    free(parametr);
  }



  
  CLOSESOCKET(listen_socket);
  CLOSESOCKET(listen_socket_data);

  


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