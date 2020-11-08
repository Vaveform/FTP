#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
// Sorry in GCC have not C11 threads support
// So i use POSIX standard
#include <pthread.h>
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

void* admin_working (void* params){
  printf("Welcome to admin CLI FTP-server\n");
  printf("Server: ");
  char some_command[256];
  fgets(some_command, 256, stdin);
  printf("Command: %s\n", some_command);
  printf("Admin CLI finishing...\n");
  pthread_exit(0);
}


int main(int argc, char** argv){
  pthread_t tid;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&tid, &attr, admin_working, NULL);
  FILE* logs = fopen("logs", "a+");
  fprintf(logs, "%s: Starting FTP Server", current_time_system());
  
  SOCKET listen_socket = create_listen_socket(argv[1], "21", 10, logs);
  if(!ISVALIDSOCKET(listen_socket)){
    fprintf(logs, "%s: Invalid listen_socket value. (%d)\n", current_time_system(), GETSOCKETERRNO());
    fclose(logs);
    return 1;
  }
  SOCKET listen_socket_data = create_listen_socket(argv[1], "20", 10, logs);
  if(!ISVALIDSOCKET(listen_socket_data)){
    fprintf(logs, "%s: Invalid listen_socket_data value. (%d)\n", current_time_system(), GETSOCKETERRNO());
    fclose(logs);
    return 1;
  }


  fprintf(logs, "%s: Waiting a connection...\n", current_time_system());
  // client_address - to store address info for the connecting client
  struct sockaddr_storage client_address;
  // To call accept and getnameinfo it is neccessary to pass size of client_address
  socklen_t client_len = sizeof(client_address);


  // Programm blocked until new input connection
  SOCKET socket_client = accept(listen_socket, (struct sockaddr*)&client_address, &client_len);

  fprintf(logs, "%s: Connected!!!\n", current_time_system());
  

  if(!ISVALIDSOCKET(socket_client)){
    fprintf(logs, "%s: accept() for listen_socket failed. (%d)\n",current_time_system(), GETSOCKETERRNO());
    CLOSESOCKET(listen_socket);
    fclose(logs);
    return 1;
  }

  fprintf(logs, "%s: Client is connected...\n", current_time_system());
  char address_buffer[100];
  getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  char buffer[1024];
  const char* invitation = 
    "Welcome to FTP server!!!\n"
    "To enter, please input login and password.\r\n";
  int bytes_send = send(socket_client, invitation, strlen(invitation), 0);
  int bytes_received = recv(socket_client, buffer, 1024, 0);
  fprintf(logs, "%s: Got login: %s", current_time_system(),buffer);
  bytes_received = recv(socket_client, buffer, 1024, 0);
  fprintf(logs, "%s: Got password: %s\n",current_time_system(), buffer);
  char generic_command[COMMAND_SIZE];
  char* command, *parametr;
  while(1){
    //printf("Circle...\n");
    bytes_received = recv(socket_client, generic_command, COMMAND_SIZE, 0);
    fprintf(logs, "%s: Received generic command: %s\n", current_time_system(), generic_command);
    parse_command(generic_command, &command, &parametr);
    //printf("Command: %s param: %s; param strlen: %d\n", command, parametr, strlen(parametr));
    int flag_operation = 1;
    if(!strcmp(command,"get")){
      if(!find_file_in_current_directory(parametr)){
        flag_operation = 0;
        send(socket_client, &flag_operation, sizeof(int), 0);
        fprintf(logs, "%s: File not found!!!\n", current_time_system());
      }
      else{
        flag_operation = 1;
        send(socket_client, &flag_operation, sizeof(int), 0);
        // Here should be error handling
        FILE* to_read = fopen(parametr, "rb");

        SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
        if(!ISVALIDSOCKET(socket_client_data)){
          fprintf(logs, "%s: accept() for listen_socket_data failed. (%d)\n",current_time_system(), GETSOCKETERRNO());
          CLOSESOCKET(listen_socket_data);
          fclose(logs);
          return 1;
        }
        fprintf(logs, "%s: Sent file with name %s to client with size: %lu bytes\n", parametr,current_time_system(), send_file_to_peer(socket_client_data, to_read, CHUNK_SIZE));
        fclose(to_read);
        CLOSESOCKET(socket_client_data);
      }
    }
    else if(!strcmp(command, "put")){
      if(find_file_in_current_directory(parametr)){
        flag_operation = 0;
        send(socket_client, &flag_operation, sizeof(int), 0);
        fprintf(logs, "%s: File already exist!!!\n", current_time_system());
      }
      else{
        flag_operation = 1;
        send(socket_client, &flag_operation, sizeof(int), 0);
        FILE* to_write = fopen(parametr, "ab");

        SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
        if(!ISVALIDSOCKET(socket_client_data)){
          fprintf(logs, "%s: accept() for listen_socket_data failed. (%d)\n", current_time_system(), GETSOCKETERRNO());
          CLOSESOCKET(listen_socket_data);
          fclose(logs);
          return 1;
        }
        fprintf(logs, "%s: Received file with name %s from client with size: %lu bytes\n", current_time_system(), parametr, recv_file_from_peer(socket_client_data, to_write, CHUNK_SIZE));
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
      fprintf(logs, "%s: Finished\n", current_time_system());
      free(command);
      free(parametr);
      break;
    }
    else{
      fprintf(logs, "%s: Command not found!!!\n", current_time_system());
      free(command);
      free(parametr);
    }
    free(command);
    free(parametr);
  }



  fclose(logs);
  CLOSESOCKET(listen_socket);
  CLOSESOCKET(listen_socket_data);
  pthread_join(tid, NULL);
  printf("Main thread finishing...\n");
  


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