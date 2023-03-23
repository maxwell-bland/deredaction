#!/usr/bin/env python3
'''
Takes in x_i, x_f, and y coordinates of the words in a pdf,
walks two PNGs along the x coordinates of each word, looking
for cases where there were no changes with respect to the
subtracted pixel differences between the PNGs, and prints
the x_i, x_f, and y values if there was no difference.
For details, consider locate.sh
'''
import sys
from PIL import Image
import numpy

WHITE_P = (0xFF, 0xFF, 0xFF)

def get_pixel_rows():
    tjs_im = Image.open(sys.argv[1])
    ppm_h = tjs_im.size[1]
    ppm_w = tjs_im.size[0]
    tjs_im_arr = numpy.frombuffer(tjs_im.tobytes(), dtype=numpy.uint8)
    tjs_rs3D = tjs_im_arr.reshape((ppm_h, ppm_w, 3))

    im = Image.open(sys.argv[2])
    im_arr = numpy.frombuffer(im.tobytes(), dtype=numpy.uint8)
    rs3D = im_arr.reshape((ppm_h, ppm_w, 3))

    return tjs_rs3D, rs3D

def is_black(im, x, y):
    p = im[y][x][:3]
    return all(p[i] <= 0xF for i in range(3))

def get_pix(im, x, y):
    p = im[y][x][:3]
    return (p[0] << 16) | (p[1] << 8) | p[2]


tjs_pix_rows, pix_rows = get_pixel_rows()

while True:
    try:
        word, x1_o, x2_o, y_o = (x for x in input().split())
        x1, x2, y = (int((float(x) / 72) * 300) for x in (x1_o, x2_o, y_o))
        x1 = x1 + 5
        x2 = x2 - 5
        y = y - 5
    except EOFError:
        break

    diff = False
    for x in range(x1,x2):
        val = get_pix(tjs_pix_rows, x, y) - get_pix(pix_rows, x, y)
        if val != 0:
            diff = True
            break

    if not diff and all(is_black(pix_rows, x,y) for x in range(x1,x2)):
        print(f'{word}\t{x1_o}\t{x2_o}\t{y_o}')
