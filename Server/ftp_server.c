#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <poll.h>
// Sorry in GCC have not C11 threads support
// So i use POSIX standard
#include <pthread.h>
#include "../net_headers.h"
#include "../strings_and_files.h"
#include "sqlite3.h"
#include "db_user_autorization.h"

// Output messages after user authorization
const char* good_result_authorization = "You are successfully login";
const char* bad_result_authorization = "You not found in server database. Please try again - 1 or registrate - 2";

// Output messages after registration
const char* good_result_registration = "You are successfully registrated";
const char* bad_result_registration = "Registration failed. Please try again - 1 or authorizate - 2";

// Output messages when user not found in registrated users
const char* after_bad_result_authorization = "If you want to registrated input login and password";




const char* unkn_command = "Unknown server command";

// Declarated database names and table with user information
const char* db_name = "users";
const char* table_name = "autorization_data";

// server_status = 1 - server running and waiting connections
// server_status = 0 - server stopped
int server_status = 1;


// Structure with server parametr to pass in other thread
typedef struct system_parametrs{
  int server_status;
  sqlite3* open_database;
  const char* name_of_table;
} system_parametrs;

// To convert string value to int (long) : (int) strtol(argv[i], (char **)NULL, 10);

#define invitation_strlen 70
const char* invitation = 
    "Welcome to FTP server!!!\n"
    "To enter, please input login and password.\r\n";



void* admin_command_interface (void* params){
  printf("Welcome to admin CLI FTP-server\n");
  system_parametrs* parametrs = params;
  char some_command[256];
  char* command, *parametr;
  while(parametrs->server_status){
    printf("Server: ");
    fgets(some_command, 256, stdin);
    
    parse_command(some_command, &command, &parametr);

    printf("Command: %s\n", command);
    if(!strcmp(command, "users")){
      printf("Total number of registred users: %lu\n", get_new_user_id());
      print_table(parametrs->open_database, parametrs->name_of_table);
    }
    else if(!strcmp(command, "stop")){
      printf("Stopping server\n");
      parametrs->server_status = 0;
    }
    else{
      printf("Invalid token\n");
    }
    free(command);
    free(parametr);
  }
  printf("Admin CLI finishing...\n");
  pthread_exit(0);
}

int abort_all_users(fd_set* users, SOCKET max_socket_){
  int number_of_users = 0;
  for(SOCKET i = 1; i <= max_socket_; i++){
    if(FD_ISSET(i, users)){
      number_of_users++;
      CLOSESOCKET(i);
      FD_CLR(i, users);
    }
  }
  return number_of_users;
}



int main(int argc, char** argv){

  // thread inditifier
  pthread_t tid;


  // Init attr
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  

  // Open database
  sqlite3* database = open_database(db_name, table_name);
  if(database == NULL){
    printf("Here\n");
    return 1;
  }
  system_parametrs parametrs;
  parametrs.server_status = server_status;
  parametrs.open_database = database;
  parametrs.name_of_table = table_name;
  pthread_create(&tid, &attr, admin_command_interface, &parametrs);

  // Configuring server
  SOCKET listen_socket = create_listen_socket(argv[1], "21", 10, NULL);
  char input_data[1024];
  char generic_command[COMMAND_SIZE];
  char* login = NULL, *password = NULL;
  char* command, *parametr;
  int flag_operation;
  fd_set connections;
  FD_ZERO(&connections);
  FD_SET(listen_socket, &connections);
  SOCKET max_socket = listen_socket;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500000;
  while(parametrs.server_status ){
    
    fd_set copy_connection = connections;
    if(select(max_socket + 1, &copy_connection, 0, 0, &tv) < 0){
      //printf("Select error\n");
      break;
    }
    for(SOCKET i = 1; i <= max_socket; i++){
      if(FD_ISSET(i, &copy_connection)){
        if(i == listen_socket){
          // Adding new user
          SOCKET new_client = accept(listen_socket, NULL, NULL);
          FD_SET(new_client, &connections);
          Client_FTP_Message output;
          output.status = IN_AUTHORIZATION;
          output.auth_token = USER_NEW_ON_SERVER;
          output.reg_token = REGISTRATION_NOT_BEGIN;
          strcpy(output.message, invitation);
          send(new_client, (void*) &output, sizeof(output), 0);
          if(new_client > max_socket)
            max_socket = new_client;
        }
        else{
          // Working with responses
          Client_FTP_Message input;
          int bytes_received = recv(i, (void*) &input, sizeof(input), 0);
          if(bytes_received < 1)
            continue;
          if(input.status == IN_AUTHORIZATION){
            // Client sent login and password
            char* login = NULL, *password = NULL;
            parse_command(input.message, &login, &password);
            memset(input.message, '\0' ,COMMAND_SIZE);
            if(login == NULL || password == NULL){
              if(login != NULL)
                free(login);
              if(password != NULL)
                free(password);
              input.auth_token = USER_NOT_FOUND_IN_SERVER_DATABASE;
              strcpy(input.message, bad_result_authorization);
              send(i, (void*) &input, sizeof(input), 0);
              continue;
            }
            if(find_concrete_user(database, table_name, login, password) == 1){
              input.auth_token = USER_FOUND_IN_SERVER_DATABASE;
              // input.status = AUTHORIZATED;
              strcpy(input.message, good_result_authorization);
              send(i, (void*) &input, sizeof(input), 0);
            }
            else{
              input.auth_token = USER_NOT_FOUND_IN_SERVER_DATABASE;
              strcpy(input.message, bad_result_authorization);
              send(i, (void*) &input, sizeof(input), 0);
            }
            free(login);
            free(password);
          }
          else if(input.status == IN_REGISTRATION){
            // Client sent login and password
            char* login = NULL, *password = NULL;
            parse_command(input.message, &login, &password);
            memset(input.message, '\0' ,COMMAND_SIZE);
            if(login == NULL || password == NULL){
              if(login != NULL)
                free(login);
              if(password != NULL)
                free(password);
              input.reg_token = REGISTRATION_FAILED;
              strcpy(input.message, bad_result_registration);
              send(i, (void*) &input, sizeof(input), 0);
              continue;
            }
            // Here deep bug in add_user - may be added equal users
            if(add_user(database, table_name, login, password, "127.0.0.1") == 0){
              input.reg_token = REGISTRATION_COMPLETE;
              strcpy(input.message, good_result_registration);
              send(i, (void*)&input, sizeof(input), 0);
            }
            else{
              input.reg_token = REGISTRATION_FAILED;
              strcpy(input.message, bad_result_registration);
              send(i, (void*)&input, sizeof(input), 0);
            }
            free(login);
            free(password);
          }
          else if(input.status == AUTHORIZATED){
            // Sending and receiving files
            char* command = NULL, *argument = NULL;
            parse_command(input.message, &command, &argument);
            memset(input.message, '\0' ,COMMAND_SIZE);
            if(command != NULL && !strcmp(command, "ls")){
              char* list_of_files = files_in_current_directory();
              strcpy(input.message, list_of_files);
              send(i, (void*)&input, sizeof(input), 0);
              free(list_of_files);
            }
            else{
              strcpy(input.message, "Invalid command");
              send(i, (void*)&input, sizeof(input), 0);
            }
            if(command != NULL)
              free(command);
            if(argument != NULL)
              free(argument);
          }
          else if(input.status == DISCONNECTING){
            FD_CLR(i, &connections);
            CLOSESOCKET(i);

          }
        }
      }
    }
  }
  
  pthread_join(tid, NULL);
  printf("Admin thread finished\n");
  abort_all_users(&connections, max_socket);
  sqlite3_close(database);
  CLOSESOCKET(listen_socket);


  // char* selectall = SELECT_ALL_FROM_TABLE("data_autoasdrization");
  // char* create_tab = CREATE_TABLE_BY_CONCRETE_PATTERN("datasdfs_autorization");
  // char* find_us = FIND_USER_BY_LOGIN_AND_PASSWORD("data_autorizatsdfion", "Dotasdfsd", "ghpsdfsdfewg");
  // char* add_us = ADD_USER_TO_DATABASE("data_ausdftorization", 13512346, "dsfvsDota", "asdsaassadcx", "127.0.0.1");
  // printf("Test select all: %s\n", selectall);
  // printf("Test create_table: %s\n", create_tab);
  // printf("Test find user: %s\n", find_us);
  // printf("Test add user: %s\n", add_us);
  // free(selectall);
  // free(create_tab);
  // free(find_us);
  // free(add_us);

  // pthread_t tid;
  // pthread_attr_t attr;
  // pthread_attr_init(&attr);
  // pthread_create(&tid, &attr, admin_working, NULL);

  
  // FILE* logs = fopen("logs", "a+");
  // fprintf(logs, "%s: Starting FTP Server", current_time_system());
  
  // SOCKET listen_socket = create_listen_socket(argv[1], "21", 10, logs);
  // if(!ISVALIDSOCKET(listen_socket)){
  //   fprintf(logs, "%s: Invalid listen_socket value. (%d)\n", current_time_system(), GETSOCKETERRNO());
  //   fclose(logs);
  //   return 1;
  // }
  // SOCKET listen_socket_data = create_listen_socket(argv[1], "20", 10, logs);
  // if(!ISVALIDSOCKET(listen_socket_data)){
  //   fprintf(logs, "%s: Invalid listen_socket_data value. (%d)\n", current_time_system(), GETSOCKETERRNO());
  //   fclose(logs);
  //   return 1;
  // }


  // fprintf(logs, "%s: Waiting a connection...\n", current_time_system());
  // // client_address - to store address info for the connecting client
  // struct sockaddr_storage client_address;
  // // To call accept and getnameinfo it is neccessary to pass size of client_address
  // socklen_t client_len = sizeof(client_address);


  // // Programm blocked until new input connection
  // SOCKET socket_client = accept(listen_socket, (struct sockaddr*)&client_address, &client_len);

  // fprintf(logs, "%s: Connected!!!\n", current_time_system());
  

  // if(!ISVALIDSOCKET(socket_client)){
  //   fprintf(logs, "%s: accept() for listen_socket failed. (%d)\n",current_time_system(), GETSOCKETERRNO());
  //   CLOSESOCKET(listen_socket);
  //   fclose(logs);
  //   return 1;
  // }

  // fprintf(logs, "%s: Client is connected...\n", current_time_system());
  // char address_buffer[100];
  // getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  // char buffer[1024];
  // const char* invitation = 
  //   "Welcome to FTP server!!!\n"
  //   "To enter, please input login and password.\r\n";
  // int bytes_send = send(socket_client, invitation, strlen(invitation), 0);
  // int bytes_received = recv(socket_client, buffer, 1024, 0);
  // fprintf(logs, "%s: Got login: %s", current_time_system(),buffer);
  // bytes_received = recv(socket_client, buffer, 1024, 0);
  // fprintf(logs, "%s: Got password: %s\n",current_time_system(), buffer);
  // char generic_command[COMMAND_SIZE];
  // char* command, *parametr;
  // while(1){
  //   //printf("Circle...\n");
  //   bytes_received = recv(socket_client, generic_command, COMMAND_SIZE, 0);
  //   fprintf(logs, "%s: Received generic command: %s\n", current_time_system(), generic_command);
  //   parse_command(generic_command, &command, &parametr);
  //   //printf("Command: %s param: %s; param strlen: %d\n", command, parametr, strlen(parametr));
  //   int flag_operation = 1;
  //   if(!strcmp(command,"get")){
  //     if(!find_file_in_current_directory(parametr)){
  //       flag_operation = 0;
  //       send(socket_client, &flag_operation, sizeof(int), 0);
  //       fprintf(logs, "%s: File not found!!!\n", current_time_system());
  //     }
  //     else{
  //       flag_operation = 1;
  //       send(socket_client, &flag_operation, sizeof(int), 0);
  //       // Here should be error handling
  //       FILE* to_read = fopen(parametr, "rb");

  //       SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
  //       if(!ISVALIDSOCKET(socket_client_data)){
  //         fprintf(logs, "%s: accept() for listen_socket_data failed. (%d)\n",current_time_system(), GETSOCKETERRNO());
  //         CLOSESOCKET(listen_socket_data);
  //         fclose(logs);
  //         return 1;
  //       }
  //       fprintf(logs, "%s: Sent file with name %s to client with size: %lu bytes\n", parametr,current_time_system(), send_file_to_peer(socket_client_data, to_read, CHUNK_SIZE));
  //       fclose(to_read);
  //       CLOSESOCKET(socket_client_data);
  //     }
  //   }
  //   else if(!strcmp(command, "put")){
  //     if(find_file_in_current_directory(parametr)){
  //       flag_operation = 0;
  //       send(socket_client, &flag_operation, sizeof(int), 0);
  //       fprintf(logs, "%s: File already exist!!!\n", current_time_system());
  //     }
  //     else{
  //       flag_operation = 1;
  //       send(socket_client, &flag_operation, sizeof(int), 0);
  //       FILE* to_write = fopen(parametr, "ab");

  //       SOCKET socket_client_data = accept(listen_socket_data, (struct sockaddr*)&client_address, &client_len);
  //       if(!ISVALIDSOCKET(socket_client_data)){
  //         fprintf(logs, "%s: accept() for listen_socket_data failed. (%d)\n", current_time_system(), GETSOCKETERRNO());
  //         CLOSESOCKET(listen_socket_data);
  //         fclose(logs);
  //         return 1;
  //       }
  //       fprintf(logs, "%s: Received file with name %s from client with size: %lu bytes\n", current_time_system(), parametr, recv_file_from_peer(socket_client_data, to_write, CHUNK_SIZE));
  //       fclose(to_write);
  //       CLOSESOCKET(socket_client_data);
  //     }
  //   }
  //   else if(!strcmp(command, "ls")){
  //     char* list_of_files = files_in_current_directory();
  //     send(socket_client, list_of_files, 1024, 0);
  //     free(list_of_files);
  //   }
  //   else if(!strcmp(command, "exit")){
  //     fprintf(logs, "%s: Finished\n", current_time_system());
  //     free(command);
  //     free(parametr);
  //     break;
  //   }
  //   else{
  //     fprintf(logs, "%s: Command not found!!!\n", current_time_system());
  //     free(command);
  //     free(parametr);
  //   }
  //   free(command);
  //   free(parametr);
  // }



  // fclose(logs);
  // CLOSESOCKET(listen_socket);
  // CLOSESOCKET(listen_socket_data);
  // pthread_join(tid, NULL);
  // printf("Main thread finishing...\n");
  
  // const char* db_name = "users";
  // const char* table_name = "autorization_data";

  // sqlite3* db = open_database(db_name, table_name);
  // if(db == NULL){
  //   puts("Error!!!\n");
  //   return 1;
  // }
  // int p = add_user(db, table_name, "Maksim", "ewrwer", "127.0.0.1");
  // printf("%d\n", p);
  // p = add_user(db, table_name, "Obama", "dsfsdfvc", "192.168.0.1");
  // printf("%d\n", p);
  // print_table(db, table_name);
  // if(find_concrete_user(db, table_name, "Maksim", "ewrwer")){
  //   puts("User Maksim found!!!\n");
  // }
  // if(!find_concrete_user(db, table_name, "Maksim", "ssssss")){
  //   puts("undifined!!!\n");
  // }
  // sqlite3_close(db);

    return 0;
}