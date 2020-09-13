#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include "./library/sparse_header.h"
#include "./library/chunk_header.h"
#include "./library/functions.h"

/**
 * This function read a file until the next chunk_header
 * int nb_blk : The number of blocks to read before the next header
 * int blk_sz : the size in bytes of a block
 * int *chunks2read : The number of chunks we didn't read yet
 * int sparse_file : The file we want to read
 * return the bytes read
 */
char *read_to_next_chunk_header_inclusive(int nb_blk, int blk_sz, int *chunks2read, int sparse_file){

    int size_buff = 0;

    if(chunks2read != 0) {
        size_buff = nb_blk*blk_sz+12;
    } else {
        size_buff = nb_blk*blk_sz;
    }

    *chunks2read = *chunks2read - 1;

    char *buffer = malloc(size_buff);
    int read_head = read(sparse_file, buffer, size_buff);
    if (read_head < 0){
        perror("error while reading file");
        exit(1);
    }

    return buffer;
}


/**
 * This function read the file header
 * int fd : The file we want to read the header
 * return the 28 bytes of the file header
 */
char *read_file_header(int fd){
    int read_nb;
    char *read = read_file(fd,FILE_HEADER_SIZE, &read_nb);
    if (read_nb != FILE_HEADER_SIZE){
        perror("Couldn't read the whole file header size");
        exit(1);
    }
    return read;
}


/**
 * This function read a chunk_header
 * int fd : The file we want to read
 * return the 12 bytes of the chunk header
 */
char *read_chunk_header(int fd){
    int read_nb;
    char *read = read_file(fd,CHUNK_HEADER_SIZE, &read_nb);
    if (read_nb != CHUNK_HEADER_SIZE){
        perror("Couldn't read the whole chunk header size");
        exit(1);
    }
    return read;
}

int main(int argc, char *argv[]) {
    int sparse_file;
    int sparse_file_tmp;
    int img_file;
    int blk_sz = 1; // Block size in byte

    if(argc == 1)
    {
        char *buffer = malloc(sizeof(char)*4000);
        int val;

        sparse_file = 0;

        sparse_file_tmp = open("tmp1.simg", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(sparse_file_tmp < 0){
            perror("Error while opening the target file");
            exit(1);
        }

        while((val = read(sparse_file, buffer, sizeof(char)*4000)) != 0){
            if(val < 0){
            perror("error while reading file");
            exit(1);
            }
            if(write(sparse_file_tmp, buffer, val) != val){
                perror("error while writing in file");
                exit(1);
            }
        }
        
        close(sparse_file_tmp);


        sparse_file = open("tmp1.simg", O_RDONLY);
        if(sparse_file < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }


        free(buffer);

        img_file = open("tmp1.img", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(img_file < 0){
            perror("Error while opening the target file");
            exit(1);
        }

    }
    else if(argc != 3){
        perror("Wrong entry !\nPlease use the program like this : ./sparse2img <sparse file> <target file>\n");
        return 0;
    }
    else {
        sparse_file = open(argv[1], O_RDONLY);
        if(sparse_file < 0){
            perror("Error while opening the target file");
            exit(1);
        }

        img_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(img_file < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }
    }

    // Reading the header of the file
    sparse_header_t *file_header = (sparse_header_t *)read_file_header(sparse_file);
    blk_sz = file_header->blk_sz;

    int chunks2read = file_header->total_chunks;

    // Reading the first chunk header
    chunk_header_t *chunk_header = (chunk_header_t *)read_chunk_header(sparse_file);

    do{

        char *buffer;
        if((int)chunk_header->chunk_type == CHUNK_TYPE_RAW){
            buffer = read_to_next_chunk_header_inclusive((int)chunk_header->chunk_sz, blk_sz, &chunks2read, sparse_file);
            retranscribe_blks(img_file, buffer, (int)chunk_header->chunk_sz*blk_sz);
            memcpy(chunk_header, (buffer + (int)chunk_header->chunk_sz*blk_sz), CHUNK_HEADER_SIZE);
        }
        else if((int)chunk_header->chunk_type == CHUNK_TYPE_FILL){
            buffer = read_to_next_chunk_header_inclusive(1, 4, &chunks2read, sparse_file);
            char buffer_tmp[(int)chunk_header->chunk_sz*blk_sz];
            for(int i = 0; i < (int)chunk_header->chunk_sz*blk_sz; i++){
                for(int j = 0; j < 4; j++){
                    buffer_tmp[i] = buffer[j];
                }
            }
            retranscribe_blks(img_file, buffer_tmp, (int)chunk_header->chunk_sz*blk_sz);
            memcpy(chunk_header, (buffer + 4), CHUNK_HEADER_SIZE);
        }
        else if((int)chunk_header->chunk_type == CHUNK_TYPE_DONT_CARE){
            buffer = read_to_next_chunk_header_inclusive(1, 0, &chunks2read, sparse_file);
            char buffer_tmp[(int)chunk_header->chunk_sz*blk_sz];
            for(int i = 0; i < ((int)chunk_header->chunk_sz*blk_sz); i++){
                    buffer_tmp[i] = 0;
            }
            retranscribe_blks(img_file, buffer_tmp, (int)chunk_header->chunk_sz*blk_sz);
            memcpy(chunk_header, buffer, CHUNK_HEADER_SIZE);
        }
        free(buffer);

    } while(chunks2read > 0);


    close(sparse_file);
    close(img_file);

    free(chunk_header);
    free(file_header);

    if(argc == 1){
        int val;
        char *buffer = malloc(sizeof(char)*4000);

        img_file = open("tmp1.img", O_RDONLY);
        if(img_file < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }

        while((val = read(img_file, buffer, sizeof(char)*4000)) != 0){
            if(val < 0){
            perror("error while reading file");
            exit(1);
            }
            if(write(1, buffer, val) != val){
                perror("error while writing in file");
                exit(1);
            }
        }

        if(remove("tmp1.img") != 0){
            perror("error while deleting file");
        }
        if(remove("tmp1.simg") != 0){
            perror("error while deleting file");
        }

        close(img_file);
        free(buffer);
    }

    return EXIT_SUCCESS;
}