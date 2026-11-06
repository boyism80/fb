import os
import sys

import openpyxl

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOURCE = os.path.join(ROOT, 'resources', 'table', 'map.xlsx')
TARGET = os.path.join(ROOT, 'scripts', 'sim', 'data', 'map_hosts.lua')


def load():
    hosts = {}
    book = openpyxl.load_workbook(SOURCE, read_only=True, data_only=True)
    for sheet in book.worksheets:
        head = None
        for row in sheet.iter_rows(values_only=True):
            if head is None:
                if row and 'host' in row and 'id' in row:
                    head = list(row)
                continue
            try:
                hosts[int(row[head.index('id')])] = int(row[head.index('host')] or 0)
            except (TypeError, ValueError):
                continue
    return hosts


def spans(hosts):
    found = []
    for map_id in sorted(hosts):
        last = found[-1] if found else None
        if last is not None and last[1] + 1 == map_id and last[2] == hosts[map_id]:
            last[1] = map_id
        else:
            found.append([map_id, map_id, hosts[map_id]])
    return found


def main():
    rows = spans(load())
    with open(TARGET, 'w', encoding='utf-8', newline='\n') as out:
        out.write('return {\n')
        for first, last, host in rows:
            out.write(f'    {{ {first}, {last}, {host} }},\n')
        out.write('}\n')
    print(len(rows), file=sys.stderr)


if __name__ == '__main__':
    main()
