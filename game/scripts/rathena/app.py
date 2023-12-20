import os
import regex

if __name__ == '__main__':
    for root, subdirs, files in os.walk('.'):
        for file in files:
            if not file.endswith('.txt'):
                continue

            path = os.path.join(root, file)
            with open(path, encoding='utf-8') as f:
                source = f.read()

                pattern = regex.compile(r'(\w+\s*)({(([^{}]+|(?2))*)})')
                matches = pattern.findall(source)
                for match in matches:
                    name = match[0].strip()
                    contents = match[1]
                    with open(f'lua/{name}.lua', 'w', encoding='utf-8') as lf:
                        lf.write(contents)