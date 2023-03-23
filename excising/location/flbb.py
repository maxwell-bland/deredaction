#!/usr/bin/env python3
'''
Find Line for Bounding Box
Input is the bottom then the top of the bounding box (the bottom will have a > y value)
Then the y coordinates of all the lines in the file.
y-coordinates that are within the bounding box boundaries will be printed out.
'''
import sys

bot = input()
top = input()
ys = []
while True:
    try:
        y = input()
    except EOFError:
        break

    if bot <= y <= top:
        print(y)
