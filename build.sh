#!/bin/bash

export cmpl='gcc -Wall -Wextra -ggdb -std=c11'

$cmpl -c image.c
$cmpl -c lzw.c
$cmpl -c bitstream.c

ar rcs libgifburger.a image.o lzw.o bitstream.o

$cmpl -o balls.out balls.c -L . -lgifburger -lm
$cmpl -o mandelbrot.out mandelbrot.c -L . -lgifburger -lm
