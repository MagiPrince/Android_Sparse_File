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

void test_write_file(int fd){
    printf("==> Executing tests of test_write_file\n");
    sparse_header_t *header = malloc(sizeof(sparse_header_t));
    header->magic = 0xed26ff3a;
    header->major_version = 0x1;
    header->minor_version = 0x0;
    header->file_hdr_sz = 0x1C;
    header->chunk_hdr_sz = 0xC;
    header->blk_sz = 0x1000;
    header->total_blks = 32;
    header->total_chunks = 32;
    header->image_checksum = 0x0;

    char *buffer = (char *)header;
    int nb_written = retranscribe_blks(fd, buffer, sizeof(sparse_header_t));
    assert(nb_written == 28);
    free(header);
    printf("    Test finish\n");
}

int main(int argc, char *argv[]){

    if (argc < 3){
        perror("Too few arguments");
        exit(1);
    }

    int sortie = open(argv[2], O_CREAT | O_WRONLY, 777);
    if(sortie < 0){
        perror("Error while opening the out file");
        exit(1);
    }

    test_write_file(sortie);

    if (close(sortie) < 0){
        perror("Error while closing the file");
        exit(1);
    }

    return 0;
}