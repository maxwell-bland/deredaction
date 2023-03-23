#!/usr/bin/env python3
'''
Takes in xi,xf, y at 72 DPI lists to STDIN, 300 DPI png as arg 1

outputs whether there is a redaction box, by attempting to walk a non-white
rectangle which extends >70% of the distance between xi and xf in pixels,
by echoing xi, xf, and y
'''
import sys
from PIL import Image
import numpy

WHITE_P = (0xFF, 0xFF, 0xFF)

def walk_right(x,y,spix,rs3D):
    w = 0
    while x < ppm_w - 1 and tuple(rs3D[y][x + 1])[:3] == spix:
        x += 1
        w += 1
    return x,y,w

def walk_left(x,y,xi,w,spix,rs3D):
    while tuple(rs3D[y][x - 1])[:3] == spix and x > 0:
        x -= 1
        if x < xi:
            w += 1
    return x,y,w

def walk_up(x, y, yi, spix, rs3D):
    while y > 0 and tuple(rs3D[y - 1][x])[:3] == spix:
        y -= 1
    # Back up if needed
    while y < yi and tuple(rs3D[y][x + 1])[:3] != spix:
        y += 1
    return x,y

def walk_down(x,y,y_bound,spix,rs3D,box):
    while x > box[0][0] and tuple(rs3D[y + 1][x])[:3] != spix:
        x -= 1
    while y < y_bound and y < ppm_h - 1 and tuple(rs3D[y + 1][x])[:3] == spix:
        y += 1
    return x,y

def down_right(x,y, rs3D, width):
    spix = tuple(rs3D[y][x])[:3]
    box = [(x,y)]
    x,y = walk_down(x,y, ppm_h - 1, spix, rs3D, box)

    # Back up if needed
    while y > box[0][1] and tuple(rs3D[y][x + 1])[:3] != spix:
        y -= 1
    if y == box[0][1]:
        raise RuntimeError('Down Fail!')

    x,y,w = walk_right(x,y,spix, rs3D)
    if x == ppm_w - 1 or w > width * 2:
        raise RuntimeError('Right Fail')

    box.append((x,y))
    return x,y,w,box

def up_right_down(x, y, rs3D, width):
    yi = y
    spix = tuple(rs3D[y][x])[:3]
    box = []

    x,y = walk_up(x,y, yi, spix, rs3D)
    if y == yi or y == 0:
        raise RuntimeError('Up Fail')

    box.append((x,y))

    x,y,w = walk_right(x, y, spix, rs3D)
    if x == ppm_w - 1 or w > width * 2:
        raise RuntimeError('Right Fail')

    while y < yi and x > box[0][0]:
        x,y = walk_down(x,y,yi,spix,rs3D,box)
        if y == ppm_h - 1:
            raise RuntimeError('Down Fail')
        x -= 1

    return x,y,w,box

def walk_box_success(x_o, x, y, width, rs3D):
    '''
    attempts to walk in a 1-colored rectangle in rs3D starting from
    x,y which covers > 50% of width and doesn't exceed any sanity
    checks
    '''
    yi = y
    xi = x
    spix = tuple(rs3D[y][x])[:3]

    gone_up = True
    up_fail = down_fail = False
    try:
        x0,y0,w0,box0 = up_right_down(x,y,rs3D, width)
    except RuntimeError:
        up_fail = True
    try:
        x1,y1,w1,box1 = down_right(x,y,rs3D, width)
        if x1 == box1[0][0] or y1 == ppm_h - 1:
            raise RuntimeError('Down Fail')
    except RuntimeError:
        down_fail = True
    if up_fail and down_fail:
        return False
    elif not up_fail and down_fail:
        x = x0; y = y0; w = w0; box = box0;
    elif up_fail and not down_fail:
        x = x1; y = y1; w = w1; box = box1;
        gone_up = False
    else:
        if w0 < w1:
            x = x1; y = y1; w = w1; box = box1;
            gone_up = False
        else:
            x = x0; y = y0; w = w0; box = box0;

    if gone_up:
        while y > box[0][1] and tuple(rs3D[y][x - 1])[:3] != spix:
            y -= 1
        box.append((x,y))
        x,y, w = walk_left(x,y,xi,w,spix,rs3D)
        if box[0][0] < x_o or box[1][0] > x_o + width or w < 0.5 * width:
            return False
        while tuple(rs3D[y - 1][x])[:3] == spix and y >= yi:
            if y == yi and x == xi:
                return box
            y -= 1
    else:
        x,y = walk_up(x,y,0, spix, rs3D)
        box[0] = (box[0][0], y)
        x,y, w = walk_left(x,y,xi,w,spix,rs3D)
        if box[0][0] < x_o or box[1][0] > x_o + width or w < 0.5 * width:
            return False
        while tuple(rs3D[y + 1][x])[:3] == spix and y <= yi:
            if y == yi and x == xi:
                return box
            y += 1

    return False

im = Image.open(sys.argv[1])
ppm_h = im.size[1]
ppm_w = im.size[0]
im_arr = numpy.frombuffer(im.tobytes(), dtype=numpy.uint8)
rs3D = im_arr.reshape((ppm_h, ppm_w, 3))
while True:
    try:
        x1_o, x2_o, y_o = (x for x in input().split())
        x1, x2, y = (int(float(x) / 72 * 300) for x in (x1_o, x2_o, y_o))
    except EOFError:
        break
    box = False
    for x in range(x1, x2):
        if all(rs3D[y][x][i] < 250 for i in range(0,3)):
            box = walk_box_success(x1, x, y, x2 - x1, rs3D)
            if box:
                break
    if box and box[1][1] - box[0][1] < 300:
        box = [list(map(lambda x: x / 300.0 * 72, b)) for b in box]
        if box[1][0] - box[0][0] > box[1][1] - box[0][1]:
          print(box[0][0], box[1][1], 
                box[1][0], box[0][1])
    else:
        print('-')
