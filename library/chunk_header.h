#include <stdlib.h>
#include <stdint.h>
#ifndef CHUNK_HEADER_H
#define CHUNK_HEADER_H

typedef struct chunk_header {
    uint16_t chunk_type;
    uint16_t reserved1;
    uint32_t chunk_sz;
    uint32_t total_sz;
} chunk_header_t;

#endif // CHUNK_HEADER_H