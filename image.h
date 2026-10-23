#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdio.h>

#include "lzw.h"
#include "bitstream.h"

typedef struct color_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct image_t
{
    uint16_t width;
    uint16_t height;

    color_t *palette;
    size_t palette_size;

    size_t frames_processed;
    uint16_t delaytime;

    uint8_t *quantized_frame;

    lzw_table_t lzw;
    bitstream_t stream;

    FILE *file;
} image_t;

void image_create(image_t *image,
        uint16_t width, uint16_t height,
        uint16_t delay, const char *filename);
void image_commit_frame(image_t *image, color_t *frame);
void image_close(image_t *image);

#endif
