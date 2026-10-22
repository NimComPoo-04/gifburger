#ifndef _LZW_H_
#define _LZW_H_

#include <stdint.h>
#include "bitstream.h"

// since max allowed code is 12 bits long
#define LZW_INVALID_CODE 0xFFFF

typedef struct lzw_bucket_t
{
    const uint8_t *key;
    size_t key_size;

    uint16_t code;

    struct lzw_bucket_t *next;
} lzw_bucket_t;

typedef struct lzw_table_t
{
    lzw_bucket_t **buckets;
    size_t size;

    uint16_t current_code;
    uint16_t initial_code;

    uint16_t code_bitsize;
} lzw_table_t;

void lzw_table_init(lzw_table_t *table, uint16_t initial_code);
void lzw_table_reset(lzw_table_t *table);
void lzw_table_deinit(lzw_table_t *table);
uint16_t lzw_table_add(lzw_table_t *table, const uint8_t *key, size_t key_size);
uint16_t lzw_table_get(lzw_table_t *table, const uint8_t *key, size_t key_size);

#define lzw_foreach(table) for(size_t __i = 0; __i < (table)->size; __i++)\
                           for(lzw_bucket_t *it = (table)->buckets[__i];\
                                   it; it = it->next)

void lzw_compress(bitstream_t *stream,
        lzw_table_t *table,
        const uint8_t *data, size_t data_size);

#endif
