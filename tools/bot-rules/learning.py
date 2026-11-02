import argparse
import json
from pathlib import Path

from lua import lua_key, lua_value

ROOT = Path(__file__).resolve().parents[2]
SPELLS = ROOT / 'server' / 'bot' / 'config' / 'load-spells.json'
BRAIN = ROOT / 'scripts' / 'brain' / 'data' / 'learning.lua'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--brain', default=str(BRAIN))
    args = parser.parse_args()

    classes = json.loads(SPELLS.read_text(encoding='utf-8'))['classes']
    lines = ['return {']
    for name in sorted(classes):
        lines.append(f'    {lua_key(name)} = {{')
        for entry in classes[name]['spells']:
            row = {
                'names': entry['names'],
                'level': entry['level'],
                'money': entry['money'],
                'materials': {material['name']: material['count'] for material in entry.get('materials', [])},
            }
            lines.append(f'        {lua_value(row)},')
        lines.append('    },')
    lines.append('}')
    Path(args.brain).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'learning: {len(classes)} classes')


if __name__ == '__main__':
    main()
