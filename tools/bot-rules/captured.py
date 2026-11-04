import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LOGS = ROOT / 'server' / 'bot' / 'logs' / 'brain'
OUTPUT = ROOT / 'scripts' / 'sim' / 'data' / 'captured_1.lua'


def lua_value(value):
    if value is None:
        return 'nil'
    if isinstance(value, bool):
        return 'true' if value else 'false'
    if isinstance(value, (int, float)):
        return repr(value)
    if isinstance(value, str):
        return '"' + value.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace('\r', '\\r') + '"'
    if isinstance(value, list):
        return '{ ' + ', '.join(lua_value(item) for item in value) + ' }'
    return '{ ' + ', '.join(f'{lua_key(key)} = {lua_value(item)}' for key, item in value.items() if item is not None) + ' }'


def lua_key(key):
    if isinstance(key, str) and key.isascii() and key.isidentifier():
        return key
    if isinstance(key, str) and key.isdigit():
        return f'[{int(key)}]'
    return f'[{lua_value(key)}]'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--logs', default=str(LOGS))
    parser.add_argument('--output', default=str(OUTPUT))
    parser.add_argument('--limit', type=int, default=5000)
    options = parser.parse_args()
    records = []
    for path in sorted(Path(options.logs).glob('*.jsonl')):
        with open(path, encoding='utf-8', errors='replace') as file:
            for line in file:
                try:
                    entry = json.loads(line)
                except ValueError:
                    continue
                forecast = entry.get('forecast') or {}
                if entry.get('kind') != 'fight' or 'snapshot' not in forecast:
                    continue
                snap = forecast.pop('snapshot')
                records.append({
                    'snap': snap,
                    'oid': entry.get('oid'),
                    'mob': entry.get('mob'),
                    'result': entry.get('result'),
                    'class': entry.get('class'),
                    'level': entry.get('level'),
                    'forecast': forecast,
                    'actual': entry.get('actual'),
                })
    records = records[-options.limit:]
    lines = ['return {', '    records = {']
    for record in records:
        lines.append(f'        {lua_value(record)},')
    lines += ['    },', '}']
    Path(options.output).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'captured: {len(records)} -> {options.output}')


if __name__ == '__main__':
    main()
