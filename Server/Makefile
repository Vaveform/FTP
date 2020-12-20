TARGET = server
CC = gcc
LDLIBS = -lpthread -ldl

.PHONY: all clean

all: $(TARGET)

clean:
			rm -rf $(TARGET) *.o
sqlite3.o: sqlite3.c sqlite3.h
			$(CC) -c sqlite3.c

db_user_authorization.o: db_user_authorization.c db_user_authorization.h sqlite3.h
			$(CC) -c db_user_authorization.c

strings_and_files.o: ../strings_and_files.c ../strings_and_files.h 
			$(CC) -c ../strings_and_files.c

net_source.o: ../net_source.c ../net_headers.h ../strings_and_files.h
			$(CC) -c ../net_source.c

ftp_server.o: ftp_server.c ../net_headers.h ../strings_and_files.h db_user_authorization.h sqlite3.h
			$(CC) -c ftp_server.c

$(TARGET): ftp_server.o net_source.o strings_and_files.o db_user_authorization.o sqlite3.o
			$(CC) -o $(TARGET) ftp_server.o net_source.o strings_and_files.o db_user_authorization.o sqlite3.o $(LDLIBS) 