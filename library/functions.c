#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#include "functions.h"
#include "sparse_header.h"
#include "chunk_header.h"

/**
 * This function read a file
 * int fd : The file we want to read
 * int size : The number of bytes to read
 * int *result : The number of bytes the system call read
 * return the bytes read
 */
char *read_file(int fd, int size, int *result){
    char *buffer = malloc(size);

    *result = read(fd, buffer, size);
    if (*result < 0){
        perror("error while reading file");
        exit(1);
    }

    return buffer;
}

/**
 * This function moves a file descriptor
 * int fd : The file descriptor we want to move
 * int nb_to_move : The offset in bytes
 */
void move_lseek(int fd, int nb_to_move){
    if(lseek(fd, (off_t)nb_to_move, SEEK_CUR) < 0){
        perror("error while moving the file cursor");
        exit(1);
    }
}

/**
 * This function reset a file descriptor to the beggining
 * int fd : The file descriptor we want to reset
 */
void move_lseek_begin(int fd){
    if(lseek(fd, (off_t)0, SEEK_SET) < 0){
        perror("error while moving the file cursor to beginning");
        exit(1);
    }
}

/**
 * This function write a buffer in the target file
 * int fd : the file to write into
 * char *buffer : the data we want to write in the file
 * int nb_byte_to_write : the number of bytes to write in the file
 * return the number of bytes written
 */
int retranscribe_blks(int fd, char* buffer, int nb_byte_to_write){
    int nb_written = write(fd, buffer, nb_byte_to_write);
    if(nb_written != nb_byte_to_write){
        perror("error while writing in file");
        exit(1);
    }

    return nb_written;
}