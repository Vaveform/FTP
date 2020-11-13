#!/bin/bash

## sqlite3.c -lpthread -ldl 
gcc  db_user_autorization.c sqlite3.c ../strings_and_files.c ../net_source.c ftp_server.c -o ftp_server -lpthread -ldl