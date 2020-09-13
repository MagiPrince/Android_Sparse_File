#include <stdlib.h>
#ifndef SPARSE_HEADER_H
#define SPARSE_HEADER_H

typedef struct sparse_header 
{
    uint32_t magic;
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t file_hdr_sz;
    uint16_t chunk_hdr_sz;
    uint32_t blk_sz;
    uint32_t total_blks;
    uint32_t total_chunks;
    uint32_t image_checksum;
} sparse_header_t;

#endif // SPARSE_HEADER_H