import codecs
import os
import io

files = [f for f in os.listdir('.')
         if os.path.isfile(f) and f != os.path.basename(__file__)]

for file in files:
    # read input file
    with io.open(file, 'r') as f:
        lines = f.read()

    # write output file
    with io.open(file, 'w', encoding='utf8') as f:
        f.write(lines)
