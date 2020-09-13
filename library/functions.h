#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define FILE_HEADER_SIZE 28
#define CHUNK_HEADER_SIZE 12
#define SPARSE_HEADER_MAGIC 0xed26ff3a
#define SPARSE_BLOCK_SIZE 4096
#define MAX_BUFFER_SIZE 409600

#define CHUNK_TYPE_RAW 0xCAC1 // 0xCAC1 -> raw
#define CHUNK_TYPE_FILL 0xCAC2 // 0xCAC2 -> fill
#define CHUNK_TYPE_DONT_CARE 0xCAC3 // 0xCAC3 -> don't care

typedef enum {
    READ_NEXT = 0,
    RETRANSCRIBE = 1,
    SET_UP = 2
}Decision;

typedef enum {
    SAME = 0,
    DIFFERENT = 1,
    UNDEFINED = 2
}Type;

typedef struct {
    int n;
    Type type;
    char blk_model[SPARSE_BLOCK_SIZE];
}Chunk;

char *read_file(int fd, int size, int *result);
void move_lseek(int fd, int nb_to_move);
void move_lseek_begin(int fd);
int retranscribe_blks(int img_file, char* buffer, int nb_blk_to_write);

#endif