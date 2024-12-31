#!/usr/bin/python

import os
import re

# public:\s?(?P<impl>#if(def|ndef)[\s\S]+?#endif)

re_namespace = re.compile('namespace (?P<namespace>.+)\s{')
re_contents = re.compile('(?P<front>.+)\s*(?P<optional>\s+:[\sa-zA-Z\(\),:]+)?{(?P<contents>[\s\S]+)}')

def extract(contents):
    queue = []
    queue.append(contents)

    while queue:
        x = queue.pop(0)
        for matched in re_contents.finditer(x):
            gd = matched.groupdict()
            yield gd['front'], gd['contents']
            queue.append(gd['contents'])

for root, dirs, files in os.walk("."):
    for file in files:
        if not file.endswith('.h'):
            continue
        
        with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
            contents = f.read()
            
            step = 0
            for front, x in extract(contents):
                step = step + 1
                if step == 3:
                    print(f'file : {os.path.join(root, file)}')
                    print(f'front : {front}')
                    print(f'contents : {x}')
