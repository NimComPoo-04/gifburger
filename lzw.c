#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "lzw.h"

#define BUCKET_COUNT 1024

void lzw_code_meta_reset(lzw_table_t *table, uint16_t initial_code)
{
    table->initial_code = initial_code;
    table->current_code = initial_code;
    table->code_bitsize = log2(initial_code - 1) + 1;
}

void lzw_table_init(lzw_table_t *table, uint16_t initial_code)
{
    table->buckets = calloc(BUCKET_COUNT, sizeof(lzw_bucket_t *));
    table->size = BUCKET_COUNT;

    lzw_code_meta_reset(table, initial_code);
}

void lzw_table_reset(lzw_table_t *table)
{
    for(size_t i = 0; i < BUCKET_COUNT; i++)
    {
        lzw_bucket_t *buck = table->buckets[i];
        while(buck)
        {
            lzw_bucket_t *next = buck->next;
            free(buck);
            buck = next;
        }
        table->buckets[i] = NULL;
    }

    lzw_code_meta_reset(table, table->initial_code);
}

void lzw_table_deinit(lzw_table_t *table)
{
    lzw_table_reset(table);
    free(table->buckets);
}

// NOTE: Do this hashing thing incrementally, instead of doing this every time.

size_t lzw_table_hash(const uint8_t *key, size_t key_size)
{
    size_t hash = 0;
    size_t pos = 1;
    for(size_t i = 0; i < key_size; i++)
    {
        hash += key[i] * pos;
        pos *= 3;
    }

    return hash;
}

uint16_t lzw_table_add(lzw_table_t *table, const uint8_t *key, size_t key_size)
{
    // FIXME: Possibly Faulty
    if(table->current_code >= (1 << table->code_bitsize))
    {
        table->code_bitsize++;
    }

    size_t hash = lzw_table_hash(key, key_size);
    size_t pos = hash % table->size;

    lzw_bucket_t *bucket = table->buckets[pos];
    lzw_bucket_t *newbucket = calloc(1, sizeof(lzw_bucket_t));

    newbucket->key = key;
    newbucket->key_size = key_size;
    newbucket->code = table->current_code++;
    newbucket->next = bucket;

    table->buckets[pos] = newbucket;

    return newbucket->code;
}

uint16_t lzw_table_get(lzw_table_t *table, const uint8_t *key, size_t key_size)
{
    if(key_size == 1)
    {
        assert(key[0] < table->initial_code);
        return key[0];
    }

    size_t hash = lzw_table_hash(key, key_size);
    size_t pos = hash % table->size;

    lzw_bucket_t *bucket = table->buckets[pos];
    while(bucket)
    {
        if(key_size == bucket->key_size && memcmp(key, bucket->key, key_size) == 0)
            return bucket->code;
        bucket = bucket->next;
    }

    return LZW_INVALID_CODE;
}

//
// TODO: This needs to get optimized, first of all we need to make the hashing
// happen incrementally, cuz this shit can handle that. thats the primary optimization
// here honestly.
//
void lzw_compress(bitstream_t *stream,
        lzw_table_t *table,
        const uint8_t *data, size_t data_size)
{
    if(data_size == 0)
        return;

    const uint8_t *ptr = data;
    size_t ptr_size = 0;

    uint16_t code = 0;

    bitstream_add(stream, table->initial_code - 2, table->code_bitsize);
    for(size_t i = 0; i < data_size; i++)
    {
        ptr_size++;
        code = lzw_table_get(table, ptr, ptr_size);
        if(code == LZW_INVALID_CODE)
        {
            uint8_t bitsize = table->code_bitsize;
            code = lzw_table_get(table, ptr, ptr_size - 1);
            bitstream_add(stream, code, bitsize);
            uint16_t newcode = lzw_table_add(table, ptr, ptr_size);

            // time to reset this shit
            if(newcode == (1 << 12) - 1)
            {
                bitstream_add(stream, table->initial_code - 2, table->code_bitsize);
                lzw_table_reset(table);
            }

            ptr = ptr + ptr_size - 1;
            ptr_size = 1;
        }
    }

    code = lzw_table_get(table, ptr, ptr_size);
    bitstream_add(stream, code, table->code_bitsize);

    bitstream_add(stream, table->initial_code - 1, table->code_bitsize);

    bitstream_flush(stream);
}
