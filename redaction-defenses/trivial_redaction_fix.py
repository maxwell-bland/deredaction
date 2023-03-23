#!/usr/bin/env python3
import sys
import fitz
import re
import subprocess
import itertools
from collections import defaultdict

def get_redacted_text(path):
    p = subprocess.Popen([f'{sys.path[0]}/trivial_redaction_loc.sh', path], stdout=subprocess.PIPE)
    p.wait()
    text = p.stdout.read().decode('utf-8')
    words_to_redact = defaultdict(list)
    for line in text.split('\n'):
        if line != '':
            # split line on repeated spaces
            words = re.split('\s+', line)
            words_to_redact[words[-1]].append(words[0])

    texts = []
    for k,v in words_to_redact.items():
        # generate all possible adjacent ordered runs of length i in the list
        for i in range(1, len(v) + 1):
            for j in range(i, len(v) + 1):
                run = v[i-1:j]
                if len(run) > 1:
                    texts.append(' '.join(run))
                texts.append(''.join(run))

    # we reverse sort by length in order to get the longest redaction first
    return sorted(texts, key=lambda s: len(s), reverse=True)
 
def redaction(path, texts):
    doc = fitz.open(path)

    for page in doc:
        for text in texts:
            areas = page.search_for(text)
            [page.add_redact_annot(area, fill = (0, 0, 0)) for area in areas]
            page.apply_redactions()
         
    new_path = path[:-4] + '.redacted.pdf'
    doc.save(new_path)
    print(new_path)
 

if __name__ == "__main__":
    path = sys.argv[1]
    redaction(path, get_redacted_text(path))
