#!/usr/bin/env python3
'''
Runs print bounding boxes and defines a default
dictionary with keys corresponding to the y coords
of each line and vals corrsponding to a nice
format of the internal data printed by PBB.
'''
from collections import defaultdict
import subprocess
import sys
import os


class BBParser:
    def __init__(self, pdfpath):

        p = subprocess.Popen([
            os.path.dirname(os.path.realpath(__file__)) +
            '/../c-src/core/get-states',
            os.path.realpath(pdfpath)
        ],
                             encoding="utf-8",
                             stdout=subprocess.PIPE)
        stdout, se = p.communicate()

        self.parts = [l.split() for l in stdout.split('\n')][:-1]

    def get_char_info(self):
        pg_num = 1
        lines = defaultdict(list)
        for l in self.parts:
            if l:
                if l[0] == 'PAGEBREAK':
                    pg_num += 1
                elif l[1] in ('CHAR', 'SPACE', 'SHIFT', 'TJ'):
                    c = l[0] if l[1] == 'CHAR' else (
                        ' ' if l[1] == 'SPACE' else '')
                    y = l[-1]
                    lines[(pg_num, y)].append({'c': c, 'x': l[3], 'x_e': l[7]})
        return lines
