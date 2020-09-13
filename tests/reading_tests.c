#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "../library/sparse_header.h"
#include "../library/chunk_header.h"
#include "../library/functions.h"

#define MAGIC 0xed26ff3a

void read_file_header_test(int fd){
    int size_read = 0;
    sparse_header_t *file_head = (sparse_header_t *) read_file(fd, FILE_HEADER_SIZE, &size_read);

    printf("==> Executing tests of read_file_header\n");
    assert(file_head->magic == MAGIC && "No Magic");

    assert(file_head->blk_sz == 1024 &&
           file_head->total_blks == 1560 &&
           file_head->total_chunks == 6 &&
           file_head->file_hdr_sz == 28 &&
           file_head->chunk_hdr_sz == 12);

    free(file_head);
    printf("    Test finish\n");
}

void read_first_chunk_header_test(int fd){
    printf("==> Executing tests of read_chunk_header\n");
    int size_read = 0;
    chunk_header_t *chunk_head = (chunk_header_t *) read_file(fd, CHUNK_HEADER_SIZE, &size_read);
    assert(chunk_head->chunk_type == CHUNK_TYPE_RAW &&
           chunk_head->chunk_sz == 100 &&
           chunk_head->total_sz == 102412);

    free(chunk_head);
    printf("    Test finish\n");
}

int main(int argc, char *argv[]){

    if (argc < 3){
        perror("Too few arguments");
        exit(1);
    }
    int sparse_file = open(argv[1], O_RDONLY);
    if(sparse_file < 0){
        perror("Error while opening the sparse file");
        exit(1);
    }

    read_file_header_test(sparse_file);
    read_first_chunk_header_test(sparse_file);

    if (close(sparse_file) < 0){
        perror("Error while closing the file");
        exit(1);
    }
    return 0;
}