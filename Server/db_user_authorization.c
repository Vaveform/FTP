#include "db_user_authorization.h"


#define first_part_length_select_all 14

#define first_part_length_create_table 13
#define third_part_length_create_table 107

#define second_part_length_find_user 15
#define fourth_part_length_find_user 16

#define first_part_length_add_user 12
#define third_part_length_add_user 52


//     const char request[] = "SELECT COUNT(*) FROM data_autorization;";
//     //const char request[] = "INSERT INTO data_autorization (user_id, login, password) VALUES (0, \"Obama\", \"rtx2080\", \"123.34.12.5\");";
//     //const char request[] = "CREATE TABLE data_autorization (user_id INTEGER PRIMARY KEY, login TEXT NOT NULL, password TEXT NOT NULL);";
//     //const char request[] = "SELECT * FROM data_autorization";

// Initial amount of registrated users
static size_t new_user_id = 0;

// Recursive translate size_t value to dynamic allocated string with values
void size_to_string(char** arr, size_t ch, size_t prev, size_t current, size_t index, size_t* ptr){
    if(!(current % prev == 0 && ch / current != 0)){
        *ptr = index;
        *arr = (char*)calloc(index + 1, sizeof(char));
        (*arr)[*ptr - index] = '0' + ch / prev;
        return;
    }
    size_to_string(arr, ch, prev * 10, current * 10, index + 1, ptr);
    (*arr)[*ptr - index] = '0' + (ch % current)/prev;
}

// Count already existing users in database
int count_callback(void *NotUsed, int argc, char **argv, char **azColName){
    increment_new_user_id();
    return 0;
}

// Function for access to values - prints values
int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s\t", argv[i]);
    }
    printf("\n");
    return 0;
}

// Increment last_user_id
void increment_new_user_id(){
    new_user_id++;
    return;
}

// Get last_user_id
size_t get_new_user_id(){
    return new_user_id;
}



// Create sql request to get all records in table
char* SELECT_ALL_FROM_TABLE(const char* table){
    char* request = NULL;
    size_t second_part_length = strlen(table);
    
    const char select_pattern[first_part_length_select_all] = "SELECT * FROM ";
    request = (char*)calloc(first_part_length_select_all + second_part_length, sizeof(char));
    strncat(request, select_pattern, first_part_length_select_all);
    strncat(request, table, second_part_length);
    return request;
}


// Create sql request create table with name table and columns: user_id INTEGER PRIMARY KEY
// login TEXT NOT FULL; password TEXT NOT FULL and ip_registration TEXT NOT FULL
char* CREATE_TABLE_BY_CONCRETE_PATTERN(const char* table){
    char* request = NULL;
    size_t second_part_length = strlen(table);
    
    const char first_part_request[first_part_length_create_table] = "CREATE TABLE ";
   
    const char third_part_request[third_part_length_create_table] =
        " (user_id INTEGER PRIMARY KEY, login TEXT NOT NULL, password TEXT NOT NULL, ip_registration TEXT NOT NULL);";
    request = (char*)calloc(first_part_length_create_table + second_part_length + third_part_length_create_table, sizeof(char));
    strncat(request, first_part_request, first_part_length_create_table);
    strncat(request, table, second_part_length);
    strncat(request, third_part_request, third_part_length_create_table);
    return request;
}


// Create sql request for finding user with given login and password
char* FIND_USER_BY_LOGIN_AND_PASSWORD(const char* table, const char* login, const char* password){
    char* request = NULL;
    char* first_part_request = SELECT_ALL_FROM_TABLE(table);
    size_t first_part_length = first_part_length_select_all + strlen(table);

    const char second_part_request[second_part_length_find_user] = " WHERE login = ";
    size_t third_part_length = strlen(login);
    
    const char fourth_part_request[fourth_part_length_find_user] = " AND password = ";
    size_t fifth_part_length = strlen(password);

    request = (char*)calloc(first_part_length + second_part_length_find_user + 
        third_part_length + fourth_part_length_find_user + fifth_part_length + 4, sizeof(char));

    strncat(request, first_part_request, first_part_length);

    free(first_part_request);

    strncat(request, second_part_request, second_part_length_find_user);
    strncat(request, "\"", 1);
    strncat(request, login, third_part_length);
    strncat(request, "\"", 1);
    strncat(request, fourth_part_request, fourth_part_length_find_user);
    strncat(request, "\"", 1);
    strncat(request, password, fifth_part_length);
    strncat(request, "\"", 1);
    // strncat(request, ";", 1);
    return request;
}

// Create sql request to add user after registration
char* ADD_USER_TO_DATABASE(const char* table, size_t _new_user_id ,const char* login, const char* password, const char* ip_address){
    char* request = NULL;

    const char first_part_request[first_part_length_add_user] = "INSERT INTO ";
    size_t second_part_length = strlen(table);

    const char third_part_request[third_part_length_add_user] =  " (user_id, login, password, ip_registration) VALUES ";
    char* forth_part_request;
    size_t forth_part_length;
    size_to_string(&forth_part_request, _new_user_id, 1, 10, 0, &forth_part_length);
    forth_part_length++;
    size_t fifth_part_length = strlen(login);
    size_t sixth_part_length = strlen(password);
    size_t seventh_part_length = strlen(ip_address);
    request = (char*)calloc(first_part_length_add_user + second_part_length + 
        third_part_length_add_user + forth_part_length + fifth_part_length + sixth_part_length + seventh_part_length + 15, sizeof(char));
    strncat(request, first_part_request, first_part_length_add_user);
    strncat(request, table, second_part_length);
    strncat(request, third_part_request, third_part_length_add_user);
    strncat(request, "(", 1);
    strncat(request, forth_part_request, forth_part_length);
    free(forth_part_request);
    strncat(request, ", \"", 3);
    strncat(request, login, fifth_part_length);
    strncat(request, "\", \"", 4);
    strncat(request, password, sixth_part_length);
    strncat(request, "\", \"", 4);
    strncat(request, ip_address, seventh_part_length);
    strncat(request, "\");", 3);
    return request;
}


// Open database with database_name or create new database. Create new table in database
// with name table_of_users if table with passed name didn't create. After this function you
// Can add users to table table_of_users by function add_user or find_user in table_of_users
sqlite3* open_database(const char* database_name, const char* table_of_users){
  sqlite3* db;
  int rc = sqlite3_open(database_name, &db);
  if(rc){
    sqlite3_close(db);
    return NULL;
  }
  char* request = SELECT_ALL_FROM_TABLE(table_of_users);
  rc = sqlite3_exec(db, request, count_callback, 0, NULL);
  if( rc ){
    free(request);
    request = CREATE_TABLE_BY_CONCRETE_PATTERN(table_of_users);
    rc = sqlite3_exec(db, request, NULL, 0, NULL);
    free(request);
    if(rc){
      sqlite3_close(db);
      return NULL;
    }
    return db;
  }
  free(request);
  return db;

} 


// Find user in database with concrete password and login
// Returned -1 - undifined poiter to db
// Return -9 - some problem with db or execute statement(SQL)
// Returned 0 - user not found
// Returned 1 - user found
int find_concrete_user(sqlite3* db, const char* table_of_users, const char* login, const char* password){
    if(db == NULL){
        return -1;
    }
    sqlite3_stmt * res;
    char* request = FIND_USER_BY_LOGIN_AND_PASSWORD(table_of_users, login, password);
    int rc = sqlite3_prepare_v2(db, request, -1, &res, 0);
    if(rc != SQLITE_OK){
        free(request);
        return -9;
    }
    sqlite3_bind_int(res,1,3);
    int step = sqlite3_step(res);
    sqlite3_finalize(res);
    free(request);
    if(step == SQLITE_ROW){
        return 1;
    }
    
    return 0;
}


// Add user to open database with login and password
// Returned -1 - undifined poiter to db
// Returned -9 - some problem with db or execute statement(SQL)
// Returned 0 - user exists
// Returned 1 - OK
int add_user(sqlite3* db, const char* table_of_users, const char* login, const char* password, const char* ip){
    if(db == NULL){
        return -9;
    }
    if(find_concrete_user(db, table_of_users, login, password) == 1)
        return 0;
    char* request = ADD_USER_TO_DATABASE(table_of_users, get_new_user_id(), login, password, ip);
    int rc = sqlite3_exec(db, request, NULL, 0, NULL);
    free(request);
    if(rc){
        return -9;
    }
    increment_new_user_id();
    return 1;
}


// Print table of users data in open data base db
void print_table(sqlite3* db, const char* table_of_users){
    if(db == NULL)
        return;
    char* request = SELECT_ALL_FROM_TABLE(table_of_users);
    int rc = sqlite3_exec(db, request, callback, 0, NULL);
    free(request);
}
