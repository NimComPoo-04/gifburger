#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "image.h"

#define PI 3.14159

#define width 512
#define height 512

color_t buffer[width * height] = {};

typedef struct bubbles_t
{
    int x;
    int y;

    int vx;
    int vy;

    int rad;

    color_t color;
} bubbles_t;

#define BUBBLE_COUNT 30

#define BUBBLE_MIN_RAD 18
#define BUBBLE_MAX_RAD 30

#define BUBBLE_MIN_SPEED (width / 32)
#define BUBBLE_MAX_SPEED (width / 26)

#define BUBBLE_OPACITY 40
#define BUBBLE_BORDER 2

#define FPS 30

bubbles_t bubbles[BUBBLE_COUNT];

void draw_bubble(int indx);
void create_bubbles()
{
    for(int i = 0; i < BUBBLE_COUNT; i++)
    {
        bubbles[i].x = width/2;
        bubbles[i].y = height/2;

        float angle = rand() % 360 * PI / 180;

        int vel = rand() % (BUBBLE_MAX_SPEED - BUBBLE_MIN_SPEED) + BUBBLE_MIN_SPEED;

        bubbles[i].vx = cos(angle) * vel;
        bubbles[i].vy = sin(angle) * vel;

        bubbles[i].rad = rand() % (BUBBLE_MAX_RAD - BUBBLE_MIN_RAD) + BUBBLE_MIN_RAD;

        bubbles[i].color.r = (rand() & 0x7F) + 0x80;
        bubbles[i].color.g = (rand() & 0x7F) + 0x80;
        bubbles[i].color.b = (rand() & 0x7F) + 0x80;
    }
}

void draw_bubble(int indx)
{
    bubbles_t bob = bubbles[indx];
 
    for(int i = -bob.rad; i < bob.rad; i++)
    {
        if(i + bob.y < 0 || i + bob.y >= height)
            continue;

        for(int j = -bob.rad; j < bob.rad; j++)
        {
            if(j + bob.x < 0 || j + bob.x >= width)
                continue;

            int x = j + bob.x;
            int y = i + bob.y;

            int pos = i * i + j * j;

            // we are inside the circle
            if(pos < bob.rad * bob.rad)
            {
                if(pos > (bob.rad - BUBBLE_BORDER) * (bob.rad - BUBBLE_BORDER))
                {
                    buffer[y * width + x] = bob.color;
                }
                else
                {
                    float t = 0.4;// fmaxf(fminf(sqrtf(pos) / bob.rad, 0.6), 0.4);

                    buffer[y * width + x].r = bob.color.r * t + buffer[y * width + x].r * (1- t);
                    buffer[y * width + x].g = bob.color.g * t + buffer[y * width + x].g * (1- t);
                    buffer[y * width + x].b = bob.color.b * t + buffer[y * width + x].b * (1- t);
                }
            }
        }
    }
}

void background_render()
{
#if 0
#define CLAMP(a, b, c) (a <= b ? b : (a >= c ? c : a))

    for(int i = 0; i < width * height; i++)
    {
        int offset = 3;

        buffer[i].r /= offset;
        buffer[i].g /= offset;
        buffer[i].b /= offset;

        buffer[i].r = CLAMP((int)(int8_t)buffer[i].r, 0, 255);
        buffer[i].g = CLAMP((int)(int8_t)buffer[i].g, 0, 255);
        buffer[i].b = CLAMP((int)(int8_t)buffer[i].b, 0, 255);
    }
#endif
    memset(buffer, 0, sizeof(buffer));
}

void update_bubbles()
{
    background_render();
    for(int i = 0; i < BUBBLE_COUNT; i++)
    {
        bubbles[i].x += bubbles[i].vx/2;
        bubbles[i].y += bubbles[i].vy/2;

        if(bubbles[i].x + bubbles[i].rad >= width)
            bubbles[i].vx = -abs(bubbles[i].vx);

        else if(bubbles[i].x - bubbles[i].rad < 0)
            bubbles[i].vx = abs(bubbles[i].vx);

        if(bubbles[i].y + bubbles[i].rad >= height)
            bubbles[i].vy = -abs(bubbles[i].vy);

        else if(bubbles[i].y - bubbles[i].rad < 0)
            bubbles[i].vy = abs(bubbles[i].vy);

        bubbles[i].x += bubbles[i].vx/2;
        bubbles[i].y += bubbles[i].vy/2;

        draw_bubble(i);
    }
}

int main(int argc, char **argv)
{
    create_bubbles();

    // just to keep this stuff deterministic
    srand(69420);

    image_t image = {};
    image_create(&image, width, height, (100 / FPS));

    for(int i = 0; i < FPS * 3; i++)
    {
        update_bubbles();
        image_commit_frame(&image, buffer);
    }

    image_close(&image);

    return 0;
}
