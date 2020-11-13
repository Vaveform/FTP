#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// Open database with database_name or create new database. Create new table in database
// with name table_of_users if table with passed name didn't create. After this function you
// Can add users to table table_of_users by function add_user or find_user in table_of_users
sqlite3* open_database(const char* database_name, const char* table_of_users);


// Increment last_user_id
void increment_new_user_id();

// Get last_user_id
size_t get_new_user_id();


// Create sql request to add user after registration
char* ADD_USER_TO_DATABASE(const char* table, size_t _new_user_id ,const char* login, const char* password, const char* ip_address);


// Create sql request for finding user with given login and password
char* FIND_USER_BY_LOGIN_AND_PASSWORD(const char* table, const char* login, const char* password);

// Create sql request create table with name table and columns: user_id INTEGER PRIMARY KEY
// login TEXT NOT FULL; password TEXT NOT FULL and ip_registration TEXT NOT FULL
char* CREATE_TABLE_BY_CONCRETE_PATTERN(const char* table);


// Create sql request to get all records in table
char* SELECT_ALL_FROM_TABLE(const char* table);


// Print table of users data in open data base db
void print_table(sqlite3* db, const char* table_of_users);

// Add user to open database with login and password
// Returned 1 - if database not opened or NULL
// Returned 0 - OK
// Returned -1 - request error
int add_user(sqlite3* db, const char* table_of_users, const char* login, const char* password, const char* ip);

// Find user in database with concrete password and login
// Returned -1 - undifined poiter to db
// Return -9 - some problem with db or execute statement(SQL)
// Returned 0 - user not found
// Returned 1 - user found
int find_concrete_user(sqlite3* db, const char* table_of_users, const char* login, const char* password);


