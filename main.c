#include <stdio.h>
#include <stdlib.h>

#include "image.h"

#define width 256
#define height 256

color_t buffer[width * height] = {};

uint8_t colors[] = {
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
    1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 0, 0, 0, 0, 1, 1, 1,
    2, 2, 2, 0, 0, 0, 0, 1, 1, 1,
    2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
};

int main(int argc, char **argv)
{
    /*
    bitstream_t bits = {};
    bitstream_add(&bits, 0xff, 8);
    bitstream_add(&bits, 0xff, 9);
    bitstream_add(&bits, 0xff, 9);
    bitstream_add(&bits, 0xff, 10);
    bitstream_add(&bits, 0xff, 10);
    bitstream_flush(&bits);

    bitstream_dump(&bits);
    */
    image_t image = {};
    image_create(&image, width, height);

# if 0
    for(int i = 0; i < height * width; i++)
            buffer[i] = (color_t){colors[i], 0, 0};
#endif

    int offset = 0;
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
                buffer[offset++] = (color_t){j % 256, i % 256, rand() % 256};

    image_commit_frame(&image, buffer);

    offset = 0;
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
                buffer[offset++] = (color_t){j % 256, i % 256, rand() % 256};

    image_commit_frame(&image, buffer);

    offset = 0;
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
                buffer[offset++] = (color_t){j % 256, i % 256, rand() % 256};

    image_commit_frame(&image, buffer);

    image_close(&image);

    return 0;
}
