#!/bin/bash

## sqlite3.c -lpthread -ldl 
gcc ../strings_and_files.c ../net_source.c ftp_server.c -o ftp_server