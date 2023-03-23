#!/usr/bin/env python3
'''
reads in the input of pts and for each y coordinate line, writes a new pdf with
that text at that y coordinate
'''
import sys
import re
import math
import fpdf

y = 0
lines = []
textline = []
space_width = 0
while True:
    pts = sys.stdin.readline()
    if not pts:
        break

    # get each line of text
    if pts[:2] == 'CH':
        ty = pts[:2]
        pts = pts[2:]
        pts = re.split(r'\s+', pts)
        if ty == 'CH':
            ch = {
                'y' : float(pts[2]),
                'x0' : float(pts[1]),
                'x1' : float(pts[3]),
                'ch' : pts[0][1:-1]
            }
            if y == 0:
                y = ch['y']
            if y != 0 and y != ch['y']:
                if space_width:
                    textline.append(space_width)
                else:
                    textline.append(math.mean([ch['x1'] - ch['x0'] for ch in textline]) / 2)
                lines.append(textline)
                space_width = 0
                textline = []
            textline.append(ch)
    elif pts[:2] == 'SP' and space_width == 0:
        pts = pts[2:]
        pts = re.split(r'\s+', pts)
        space_width = (float(pts[3]) - float(pts[1])) / 2

if space_width:
    textline.append(space_width)
else:
    textline.append(math.mean([ch['x1'] - ch['x0'] for ch in textline]) / 2)
lines.append(textline)

pdf = fpdf.FPDF()
pdf.add_page()

for line in lines:
    sp_width = line[-1]
    line = line[:-1]
    min_x = min([ch['x0'] for ch in line])
    y = line[0]['y']
    text = ''
    for i in range(len(line) - 1):
        cur_c = line[i]
        next_c = line[i+1]
        distance = next_c['x0'] - cur_c['x1']
        spaces = 0
        if distance > sp_width:
            spaces = int((distance / 2 + sp_width) / sp_width)
        text += cur_c['ch'] + ' ' * spaces
    text += next_c['ch']
    # in mm
    pdf.set_xy(25.4, (y / 72) * 25.4)
    pdf.set_font('courier', 'B', 6.0)
    pdf.cell(ln=0, h=5.0, align='L', w=0, txt=text, border=0)

pdf.output(sys.argv[1], 'F')
