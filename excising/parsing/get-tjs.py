#!/usr/bin/env python3
'''
This script runs print-bounding-boxes, on the supplied PDF and then
parses it out into a set of nicely-fomatted TJ operators without 
ascii CID values.
'''
import sys
import os
import bb_parser as bbp
from collections import defaultdict

class ExtractTJ(bbp.BBParser):
    def __init__(self, *args, **kwargs):
        super(ExtractTJ, self).__init__(*args, **kwargs)

    def vector_strings(self):
        pg_num = 0
        vs = []
        lines = defaultdict(list)
        for l in self.parts:
            if l:
                if l[0] == 'PAGEBREAK':
                    pg_num += 1
                elif l[1] in ('CHAR', 'SPACE', 'SHIFT', 'TJ'):
                    y = l[8]
                    val = l[0] if l[1] == 'CHAR' else (' ' if l[1] == 'SPACE' else float(l[0]))
                    lines[(pg_num, y)].append({ 'c': val, 'x': l[3] })

        make_chr = lambda x, f: f(x) if (type(x['c']) is not str) else x['c']
        for k, v in lines.items():
            s = ''
            adj_s = '['
            for x in v:
                if not x['c'] and s != '':
                    adj_s += ']'
                    vs.append([k[0], k[1], adj_s, s])
                    adj_s = '['
                    s = ''
                elif x['c'] != 0:
                        adj_s += make_chr(x, lambda s: '(' + ('%0.6f' % x['c']) + ')')
                        s += make_chr(x, lambda s: '')
            adj_s += ']'
            vs.append([k[0], k[1], adj_s, s])

        return vs


if __name__ == '__main__':
    etj = ExtractTJ(os.path.realpath(sys.argv[1]))
    print('page,y,vectstr,strn')
    for v in etj.vector_strings():
        print(str(v[0]) + ',' + str(v[1]) + ',|' + v[2] + '|,|' + v[3] + '|')
