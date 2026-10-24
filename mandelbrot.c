#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"

#define width 640// goes from -320/240 to 320/240
#define height 480// goes from -1 to 1

#define MAX_ITER (1024 + 64)
#define FPS 24

image_t image = {};
color_t buffer[width * height] = {};

int escape_iters(double x, double y)
{
    double zx = 0;
    double zy = 0;

    int iterations = 0;
    while(zx * zx + zy * zy < 2 && iterations < MAX_ITER)
    {
        double tmp = zx * zx - zy * zy + x;
        zy = 2 * zx * zy + y;
        zx = tmp;
        
        iterations++;
    }

    return iterations;
}

void render_frame(double x, double y, double zoom)
{
    double yy = y - 1.0/zoom;
    double dy = 2.0/zoom / height;

    double xx = x - width/zoom/height;
    double dx = 2.0/zoom/height;

    for(int i = 0, offset = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            int count = escape_iters(xx, yy) % 256; 
            buffer[offset++] = (color_t){count, 32, 64};//image.palette[count];

            xx += dx;
        }
        xx = x - width/zoom/height;
        yy += dy;
    }
}

int main(void)
{
    image_create(&image, width, height, 100/FPS, "test.gif");

    double min_zoom = 0.25;
    double max_zoom = 24000;

    double b_a = log(max_zoom/min_zoom);
    double a = log(min_zoom);

    int count = FPS * 6;

    double t = 0;
    double dt = 3.14159 / 2/count;

    for(int i = 0; i < count; i++)
    {
        double zoom = exp(b_a * sin(t) + a);//(max_zoom - min_zoom) * t * t + min_zoom;

        render_frame(-0.743640887037, 0.131825904205, zoom);
        //render_frame(-0.1528, 1.0397, zoom);
        //render_frame(0, 0, zoom);
        image_commit_frame(&image, buffer);

        t += dt;

        printf("%3d frame processed out of %d frames\n", i + 1, count);
    }
    image_close(&image);
}
