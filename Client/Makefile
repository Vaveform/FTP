TARGET = ftp_client
CC = gcc

.PHONY: all clean

all: $(TARGET)

clean:
			rm -rf $(TARGET) *.o

strings_and_files.o: ../strings_and_files.c ../strings_and_files.h 
			$(CC) -c ../strings_and_files.c

net_source.o: ../net_source.c ../net_headers.h ../strings_and_files.h
			$(CC) -c ../net_source.c

client.o: client.c ../net_headers.h ../strings_and_files.h
			$(CC) -c client.c

$(TARGET): client.o net_source.o strings_and_files.o
			$(CC) -o $(TARGET) client.o net_source.o strings_and_files.o