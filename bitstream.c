#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bitstream.h"

void bitstream_flush(bitstream_t *stream)
{
    // there is nothing to flush in the buffer man
    if(stream->pos == 0)
        return;

    if(stream->length >= stream->size)
    {
        stream->size += (stream->size ? stream->size / 2 : 16);
        stream->emitted = realloc(stream->emitted, stream->size);
    }

    stream->emitted[stream->length] = stream->buffer;
    stream->length++;

    stream->pos = 0;
    stream->buffer = 0;
}

void bitstream_add(bitstream_t *stream, uint16_t code, uint16_t size)
{
    uint8_t count = size;
    while(count)
    {
        uint8_t bit = code & 1;
        code >>= 1;

        stream->buffer |= (bit << (uint8_t)stream->pos);
        stream->pos++;

        if(stream->pos == 8)
        {
            bitstream_flush(stream);
        }

        count--;
    }
}

// NOTE: we wanna reset when we know we will dump data of similar size
// again and want to reuse the bitstream.
void bitstream_reset(bitstream_t *stream)
{
    stream->buffer = 0;
    stream->pos = 0;
    stream->length = 0;
}

void bitstream_delete(bitstream_t *stream)
{
    bitstream_reset(stream);
    stream->size = 0;
    free(stream->emitted);
}

void dump_binary(uint8_t value)
{
    for(int i = 8; i; i--)
    {
        putchar((value & 1) + '0');
        value >>= 1;
    }
}

void bitstream_dump(bitstream_t *stream)
{
    printf("Output Stream:\n");
    for(uint32_t i = 0; i < stream->length; i++)
    {
        dump_binary(stream->emitted[i]);
        printf(" %02x\n", stream->emitted[i]);
    }
    puts("");
}
