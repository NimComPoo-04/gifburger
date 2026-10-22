#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include <stdint.h>

typedef struct bitstream_t
{
    uint8_t buffer;
    uint8_t pos;

    uint8_t *emitted;
    uint32_t size;
    uint32_t length;
} bitstream_t;

void bitstream_add(bitstream_t *stream, uint16_t code, uint16_t size);
void bitstream_flush(bitstream_t *stream);
void bitstream_reset(bitstream_t *stream);
void bitstream_delete(bitstream_t *stream);

void bitstream_dump(bitstream_t *stream);

#endif
