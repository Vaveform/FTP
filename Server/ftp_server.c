#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
// Sorry in GCC have not C11 threads support
// So i use POSIX standard
#include <pthread.h>
#include "../net_headers.h"
#include "../strings_and_files.h"
#include "sqlite3.h"

static long last_user_id = 0;

// To convert string value to int (long) : (int) strtol(argv[i], (char **)NULL, 10);

// Function for access to values
int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
      if(i == argc - 1)
        printf("%s", argv[i]);
      else
        printf("%s,\t", argv[i]);
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

sqlite3* open_database(const char* database_name, const char* user_data_table_name){
  sqlite3* db;
  char *ErrMsg = NULL;
  int rc = sqlite3_open(database_name, &db);
  if(rc){
    sqlite3_close(db);
    return NULL;
  }
  const size_t request_s = 14;
  const char find_table_request[request_s] = "SELECT * FROM ";
  size_t input_table_name = strlen(user_data_table_name);
  char* full_request = (char*)calloc(request_s + input_table_name, sizeof(char));
  strncat(full_request, find_table_request, request_s);
  strncat(full_request, user_data_table_name, input_table_name);
  printf("Request: %s\n", full_request);
  rc = sqlite3_exec(db, full_request, callback, 0, &ErrMsg);
  if( rc ){
    free(full_request);
    const size_t first_part = 13;
    const char create_table_request[first_part] = "CREATE TABLE ";
    const size_t third_part = 76;
    const char table_configuration[third_part] = " (user_id INTEGER PRIMARY KEY, login TEXT NOT NULL, password TEXT NOT NULL);";
    
    char* full_request = (char*)calloc(first_part + third_part + input_table_name, sizeof(char));
    strncat(full_request, create_table_request, first_part);
    strncat(full_request,user_data_table_name, input_table_name);
    strncat(full_request, table_configuration, third_part);
    printf("Request: %s\n", full_request);
    rc = sqlite3_exec(db, full_request, callback, 0, &ErrMsg);
    free(full_request);
    if(rc){
      sqlite3_close(db);
      return NULL;
    }
    return db;
  }
  free(full_request);
  return db;

} 


long current_users_number(sqlite3* db, const char* table_name){
  if(db!= NULL){
    const size_t first_part = 21;
    const char request[first_part] = "SELECT COUNT(*) FROM ";
    size_t second_part = strlen(table_name); 
    char* full_request = (char*)calloc(first_part + second_part, sizeof(char));

    sqlite3_exec(db , "SELECT COUNT(*) FROM")
  }

}

sqlite3* add_user(sqlite3* db, const char* login, const char* password){

}

void find_table(){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("users", &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
    }
    const char request[] = "SELECT COUNT(*) FROM data_autorization;";
    //const char request[] = "INSERT INTO data_autorization (user_id, login, password) VALUES (0, \"Obama\", \"rtx2080\");";
    //const char request[] = "CREATE TABLE data_autorization (user_id INTEGER PRIMARY KEY, login TEXT NOT NULL, password TEXT NOT NULL);";
    //const char request[] = "SELECT * FROM data_autorization";
    rc = sqlite3_exec(db, &request, callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
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
  


  // sqlite3* db = open_database("users", "autorization_data");
  // if(db == NULL)
  //   puts("Error!!!\n");
  // sqlite3_close(db);

    return 0;
}