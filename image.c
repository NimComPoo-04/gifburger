#include <stdlib.h>
#include <assert.h>
#include "image.h"

// TODO: do some kmeans stuff or something
void image_palette_gen(image_t *image)
{
    image->palette_size = 8 * 8 * 4;
    image->palette = calloc(image->palette_size, sizeof(color_t));

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            for(int k = 0; k < 8; k++)
            {
                image->palette[i * 8 * 8 + j * 8 + k] = (color_t){
                    k * 32, j * 32, i * 64
                };
            }
        }
    }
}

// TODO: Do something like floyd steinberg dithering or something else
void image_quantize(image_t *image, color_t *frame)
{
#define C(a) ((a) < 0 ? 0 : ((a) > 255 ? 255 : (a)))

    for(int i = 0; i < image->height; i++)
    {
        for(int j = 0; j < image->width; j++)
        {
            int where = i * image->width + j;
            color_t col = frame[where];

            uint8_t ir = col.r / 32;
            uint8_t ig = col.g / 32;
            uint8_t ib = col.b / 64;

            int er = col.r - ir * 32;
            int eg = col.g - ig * 32;
            int eb = col.b - ib * 64;

            if(j + 1 < image->width)
            {
                frame[where + 1].r = C(frame[where + 1].r + er * 7 / 16);
                frame[where + 1].g = C(frame[where + 1].g + eg * 7 / 16);
                frame[where + 1].b = C(frame[where + 1].b + eb * 7 / 16);
            }
            
            if(i + 1 < image->height)
            {
                int w = image->width;
                if(j - 1 < 0)
                {
                    frame[where + w - 1].r = C(frame[where + w - 1].r + er * 3 / 16);
                    frame[where + w - 1].g = C(frame[where + w - 1].g + eg * 3 / 16);
                    frame[where + w - 1].b = C(frame[where + w - 1].b + eb * 3 / 16);
                }

                frame[where + w].r = C(frame[where + w].r + er * 5 / 16);
                frame[where + w].g = C(frame[where + w].g + eg * 5 / 16);
                frame[where + w].b = C(frame[where + w].b + eb * 5 / 16);

                if(j + 1 < image->width)
                {
                    frame[where + w + 1].r = C(frame[where + w + 1].r + er * 1 / 16);
                    frame[where + w + 1].g = C(frame[where + w + 1].g + eg * 1 / 16);
                    frame[where + w + 1].b = C(frame[where + w + 1].b + eb * 1 / 16);
                }
            }

            uint8_t color_indx = ib * 8 * 8 + ig * 8 + ir;
            image->quantized_frame[where] = color_indx;
        }
    }
}

void image_create(image_t *image,
        uint16_t width, uint16_t height,
        uint16_t delay, const char *filename)
{
    image->width = width;
    image->height = height;
    image_palette_gen(image);
    image->frames_processed = 0;
    image->delaytime = delay;
    image->quantized_frame = calloc(width * height, sizeof(uint8_t));
    lzw_table_init(&image->lzw, 258);
    image->file = fopen(filename, "wb+");
    assert(image->file);

    const char header[] = "GIF89a";
    fwrite(header, sizeof(header) - 1, 1, image->file);

    fwrite(&width, sizeof(width), 1, image->file);
    fwrite(&height, sizeof(height), 1, image->file);

    // we would need to change the last 3 bits and color res bits
    const uint8_t packed = 0xFF;
    fwrite(&packed, sizeof(packed), 1, image->file);

    // background color is not used, and pixel aspect ratio is 0 cuz pixels are 1:1
    uint8_t zeros[] = {0, 0};
    fwrite(zeros, sizeof(zeros), 1, image->file);

    // Global Color Table
    fwrite(image->palette, image->palette_size * sizeof(color_t), 1, image->file);

    fflush(image->file);
}

void image_commit_frame(image_t *image, color_t *frame)
{
    image_quantize(image, frame);
    lzw_table_reset(&image->lzw);
    bitstream_reset(&image->stream);

#if 1

    // Netscape application extention for looping forever
    if(image->frames_processed == 0)
    {
        const char netscape_application_ext[] = "\x21\xff\x0bNETSCAPE2.0\x03\x01\x00\x00\x00";
        fwrite(netscape_application_ext, sizeof(netscape_application_ext) - 1, 1, image->file);
    }


    // Graphics extention
    uint8_t gfx_control_ext[] = {0x21, 0xF9, 0x04, 0x00};
    fwrite(gfx_control_ext, sizeof(gfx_control_ext), 1, image->file);

    if(image->frames_processed == 0)
    {
        uint16_t delay = 0;
        fwrite(&delay, sizeof(delay), 1, image->file);
    }
    else
    {
        fwrite(&image->delaytime, sizeof(image->delaytime), 1, image->file);
    }

    // Transparency is not used and the last one is a block terminator 0
    uint8_t zeros[] = {0, 0};
    fwrite(&zeros, sizeof(zeros), 1, image->file);

#endif

    // Actual Image data
    uint8_t hdr = 0x2C;
    uint16_t x = 0, y = 0;
    fwrite(&hdr, sizeof(hdr), 1, image->file);
    fwrite(&x, sizeof(x), 1, image->file);
    fwrite(&y, sizeof(y), 1, image->file);

    fwrite(&image->width, sizeof(image->width), 1, image->file);
    fwrite(&image->height, sizeof(image->height), 1, image->file);

    // We don't have a LCT
    uint8_t packed = 0;
    fwrite(&packed, sizeof(packed), 1, image->file);

    uint8_t min_code_size = image->lzw.code_bitsize - 1;

    lzw_compress(&image->stream, &image->lzw,
            image->quantized_frame, image->width * image->height);

    //printf("LZW Bit Size: %d Min Code Size: %d\n", image->lzw.code_bitsize, min_code_size);

    // Just use the info available lol
    fwrite(&min_code_size, sizeof(uint8_t), 1, image->file);

    // We write out 255 byte chunks along with the number of shit spewed so thats 256 bytes.
    size_t bytes_left = image->stream.length;
    uint8_t *emitted = image->stream.emitted;
    while(bytes_left)
    {
        uint8_t to_write = 255;
        if(bytes_left < 255)
        {
            to_write = bytes_left;
        }

        fwrite(&to_write, sizeof(uint8_t), 1, image->file);
        fwrite(emitted, sizeof(uint8_t), to_write, image->file);

        emitted += to_write;
        bytes_left -= to_write;
    }

    uint8_t block_terminate = 0;
    fwrite(&block_terminate, sizeof(block_terminate), 1, image->file);

    fflush(image->file);

    image->frames_processed++;
}

void image_close(image_t *image)
{
    bitstream_delete(&image->stream);
    lzw_table_deinit(&image->lzw);

    free(image->palette);
    free(image->quantized_frame);

    const char trailer[] = ";";
    fwrite(trailer, sizeof(trailer) - 1, 1, image->file);
    fclose(image->file);
}
