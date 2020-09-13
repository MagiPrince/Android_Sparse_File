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
 * This function initialize a chunk
 * return the chunk initialized
 */
Chunk init_chunk(){
    Chunk chunk;
    chunk.n = -1;
    chunk.type = UNDEFINED;
    return chunk;
}

/**
 * This function reset a buffer
 * char *buffer : the buffer to reset
 */
void empty_buffer(char *buffer){
    memset(buffer, 0, MAX_BUFFER_SIZE);
}

/**
 * This function checks if a buffer is full
 * int n : the number of blocks in the buffer
 * return if the buffer is full
 */
int is_full(int n){
    return n*SPARSE_BLOCK_SIZE == MAX_BUFFER_SIZE;
}

/**
 * This function checks the block model of a chunk
 * char blk_model : the block model of a chunk
 * return the chunk_type
 */
uint16_t check_blk_model(char blk_model[]) {
    char c_test = 0;
    for(int i=0; i < SPARSE_BLOCK_SIZE; i++)
    {
        if(c_test != blk_model[i]){
            return CHUNK_TYPE_FILL;
        }
    }
    return CHUNK_TYPE_DONT_CARE;
}

/**
 * This function write a file header
 * int fd : The file where we write it
 * int total_blks : The number of blocks in the file
 * int total_chunks : The number of chunks in the file
 */
void write_file_header(int fd, int total_blks, int total_chunks){

    sparse_header_t *sparse_header = malloc(sizeof(sparse_header_t));

    sparse_header->magic = SPARSE_HEADER_MAGIC;
    sparse_header->major_version = 0x1;
    sparse_header->minor_version = 0x0;
    sparse_header->file_hdr_sz = 0x1C;
    sparse_header->chunk_hdr_sz = 0xC;
    sparse_header->blk_sz = 0x1000;
    sparse_header->total_blks = total_blks;
    sparse_header->total_chunks = total_chunks;
    sparse_header->image_checksum = 0x0;

    char *buffer = (char*)sparse_header;

    retranscribe_blks(fd, buffer, sizeof(sparse_header_t));

    free(sparse_header);

}

/**
 * This function write a chunk header
 * int fd : The file where we write it
 * Chunk *chunk : The chunk corresponding to the chunk header we write
 */
void write_chunk_header(int fd, Chunk *chunk){

    chunk_header_t *chunk_header = malloc(sizeof(chunk_header_t));

    if(chunk->type == DIFFERENT){
        move_lseek(fd, -(chunk->n * SPARSE_BLOCK_SIZE + CHUNK_HEADER_SIZE));
    }
    else{
        move_lseek(fd, -(CHUNK_HEADER_SIZE));
    }

    if(chunk->type != SAME){
        chunk_header->chunk_type = CHUNK_TYPE_RAW;
    }
    else{
        chunk_header->chunk_type = check_blk_model(chunk->blk_model);
    }
    chunk_header->reserved1 = 0;
    chunk_header->chunk_sz = chunk->n;
    chunk_header->total_sz = chunk->n*SPARSE_BLOCK_SIZE+CHUNK_HEADER_SIZE;

    char *buffer = (char*)chunk_header;

    retranscribe_blks(fd, buffer, sizeof(chunk_header_t));

    if(chunk->type == DIFFERENT){
        move_lseek(fd, chunk->n*SPARSE_BLOCK_SIZE);
    }
    else if(chunk_header->chunk_type == CHUNK_TYPE_FILL){
        retranscribe_blks(fd, chunk->blk_model, SPARSE_BLOCK_SIZE);
    }

    free(chunk_header);
}

/**
 * This function write an empty chunk header
 * int fd : The file where we write it
 */
void write_empty_chunk_header(int fd){

    chunk_header_t *chunk_header = malloc(sizeof(chunk_header_t));

    chunk_header->chunk_type = 0;
    chunk_header->reserved1 = 0;
    chunk_header->chunk_sz = 0;
    chunk_header->total_sz = 0;

    char *buffer = (char*)chunk_header;

    retranscribe_blks(fd, buffer, sizeof(chunk_header_t));

    free(chunk_header);
}

/**
 * This function read a block from a file
 * int fd : The file we want to read
 * char *read_nb : The number of bytes read
 * return the block read
 */
char *read_block(int fd, int *read_nb){
    return read_file(fd, SPARSE_BLOCK_SIZE, read_nb);
}

/**
 * This function checks if two blocks are similar
 * char *blk : A block
 * char *second_blk : Another block
 * return if the blocks are the same
 */
int same_blks(char *blk, char *second_blk){
    for (int i=0;i<SPARSE_BLOCK_SIZE;i++){
        if (blk[i] != second_blk[i]){
            return 0;
        }
    }
    return 1;
}

/**
 * This function copies a block into another
 * char *src : The block to copy
 * char *dst : The block where we copy
 */
void copy_blk(char *src, char *dst){
    for (int i=0;i<SPARSE_BLOCK_SIZE;i++){
        dst[i] = src[i];
    }
}

/**
 * This function decides what to do with a chunk and the last block read
 * char *last_blk : the last block read
 * Chunk chunk : the current chunk
 * return the decision
 */
Decision what_to_do(char *last_blk,Chunk *chunk){
    if (chunk->type == UNDEFINED){
        return SET_UP;
    }
    if (chunk->type == SAME && same_blks(last_blk,chunk->blk_model)){
        return READ_NEXT;
    }
    if (chunk->type == DIFFERENT && !same_blks(last_blk,chunk->blk_model)){
        return READ_NEXT;
    }
    return RETRANSCRIBE;
}

int main(int argc, char *argv[]) {

    int sparse_file;
    int img_file;
    int img_file_tmp;

    if(argc == 1)
    {
        char *buffer = malloc(sizeof(char)*4000);
        int val;

        img_file = 0;

        img_file_tmp = open("tmp.img", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(img_file_tmp < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }

        while((val = read(img_file, buffer, sizeof(char)*4000)) != 0){
            if(val < 0){
            perror("error while reading file");
            exit(1);
            }
            if(write(img_file_tmp, buffer, val) != val){
                perror("error while writing in file");
                exit(1);
            }
        }
        
        close(img_file_tmp);


        img_file = open("tmp.img", O_RDONLY);
        if(img_file < 0){
            perror("Error while opening the target file");
            exit(1);
        }


        free(buffer);

        sparse_file = open("tmp.simg", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(sparse_file < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }
    }
    else if(argc != 3){
        perror("Wrong entry !\nPlease use the program like this : ./img2sparse <target file> <sparse file>\n");
        return 0;
    }
    else{
        img_file = open(argv[1], O_RDONLY);
        if(img_file < 0){
            perror("Error while opening the target file");
            exit(1);
        }

        sparse_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(sparse_file < 0){
            perror("Error while opening the sparse file");
            exit(1);
        }
    }


    Chunk chunk = init_chunk();

    write_file_header(sparse_file, 0, 0);
    write_empty_chunk_header(sparse_file);

    char *datas_buffer = malloc(MAX_BUFFER_SIZE);
    char *read_buffer;

    int read_nb = SPARSE_BLOCK_SIZE;
    int total_chunks = 0; 
    int total_blks = 0;

    while (read_nb == SPARSE_BLOCK_SIZE){
        read_buffer = read_block(img_file, &read_nb);

        switch(what_to_do(read_buffer, &chunk)){
        
            case SET_UP :
                if (chunk.n == 0){
                    if(same_blks(read_buffer, chunk.blk_model)){
                        chunk.type = SAME;
                    }
                    else {
                        copy_blk(chunk.blk_model, datas_buffer);
                        chunk.type = DIFFERENT;
                    }
                }
                copy_blk(read_buffer, chunk.blk_model);
                chunk.n++;
                break;

            case RETRANSCRIBE :
                total_chunks +=1;
                total_blks += chunk.n;
                if (chunk.type == DIFFERENT){
                    int size = chunk.n % (MAX_BUFFER_SIZE/SPARSE_BLOCK_SIZE);
                    if (size != 0){
                        retranscribe_blks(sparse_file, datas_buffer, size*SPARSE_BLOCK_SIZE);
                        empty_buffer(datas_buffer);
                    }
                    write_chunk_header(sparse_file, &chunk);
                    chunk.type = SAME;
                    chunk.n = 2;
                }
                else {
                    write_chunk_header(sparse_file, &chunk);                    
                    chunk.type = UNDEFINED;
                    chunk.n = 0;
                    copy_blk(read_buffer, chunk.blk_model);
                }
                write_empty_chunk_header(sparse_file);
                break;

            case READ_NEXT :
                if (chunk.type == DIFFERENT){
                    int size = chunk.n % (MAX_BUFFER_SIZE/SPARSE_BLOCK_SIZE);
                    copy_blk(chunk.blk_model, datas_buffer + size * SPARSE_BLOCK_SIZE);
                    copy_blk(read_buffer, chunk.blk_model);
                    if (is_full(size+1)){
                        retranscribe_blks(sparse_file, datas_buffer, (size+1)*SPARSE_BLOCK_SIZE);
                        empty_buffer(datas_buffer);
                    }
                }
                chunk.n+=1;                
                break;

            default :
                printf("Uncorrect decision !\n");
                return EXIT_FAILURE;
        }
        free(read_buffer);
    }
    if (read_nb != 0){
        if (chunk.type == UNDEFINED){
            chunk.n++;
            retranscribe_blks(sparse_file, chunk.blk_model, chunk.n*SPARSE_BLOCK_SIZE);
            write_chunk_header(sparse_file, &chunk);        
        }
        else if (chunk.type == DIFFERENT){
            int size = chunk.n % (MAX_BUFFER_SIZE/SPARSE_BLOCK_SIZE);
            copy_blk(chunk.blk_model, datas_buffer + size * SPARSE_BLOCK_SIZE);
            chunk.n++;            
            retranscribe_blks(sparse_file, datas_buffer, (size+1)*SPARSE_BLOCK_SIZE);
            write_chunk_header(sparse_file, &chunk);        
        }
        else{
            write_chunk_header(sparse_file, &chunk);
        }
        total_chunks += 1;
        total_blks += chunk.n;
    }
    else if(chunk.n != 0){
        if (chunk.type == DIFFERENT){
            int size = chunk.n % (MAX_BUFFER_SIZE/SPARSE_BLOCK_SIZE);
            retranscribe_blks(sparse_file, datas_buffer, size*SPARSE_BLOCK_SIZE);
            write_chunk_header(sparse_file, &chunk);        
        }
        else{
            chunk.n -=1;
            write_chunk_header(sparse_file, &chunk);
        }
        total_chunks += 1;
        total_blks += chunk.n;
    }

    move_lseek_begin(sparse_file);
    write_file_header(sparse_file, total_blks, total_chunks);

    close(sparse_file);
    close(img_file);

    free(datas_buffer);

    if(argc == 1){
        int val;
        char *buffer = malloc(sizeof(char)*4000);

        sparse_file = open("tmp.simg", O_RDONLY);
        if(sparse_file < 0){
            perror("Error while opening the target file");
            exit(1);
        }

        while((val = read(sparse_file, buffer, sizeof(char)*4000)) != 0){
            if(val < 0){
            perror("error while reading file");
            exit(1);
            }
            if(write(1, buffer, val) != val){
                perror("error while writing in file");
                exit(1);
            }
        }

        if(remove("tmp.img") != 0){
            perror("error while deleting file");
        }
        if(remove("tmp.simg") != 0){
            perror("error while deleting file");
        }

        close(sparse_file);
        free(buffer);

    }

    return EXIT_SUCCESS;
}