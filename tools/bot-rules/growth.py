import argparse
import json
from pathlib import Path

from lua import lua_key, lua_value

ROOT = Path(__file__).resolve().parents[2]
ABILITY_JSON = ROOT / 'server' / 'game' / 'json' / 'ability.json'
LOGIN_CONFIG = ROOT / 'server' / 'login' / 'config' / 'config.dev.json'
BRAIN = ROOT / 'scripts' / 'brain' / 'data' / 'growth.lua'
STAT_BASE = 3
LEVEL_RANDOM = 4.5
FIELDS = (('hp', 'hp'), ('mp', 'mp'), ('str', 'strength'), ('int', 'intelligence'), ('dex', 'dexterity'))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--brain', default=str(BRAIN))
    args = parser.parse_args()

    table = json.loads(ABILITY_JSON.read_text(encoding='utf-8'))
    init = json.loads(LOGIN_CONFIG.read_text(encoding='utf-8'))['init']
    result = {}
    for name, rows in table.items():
        if name == 'NONE':
            continue
        current = {'hp': init['hp']['base'], 'mp': init['mp']['base'], 'str': STAT_BASE, 'int': STAT_BASE,
                   'dex': STAT_BASE, 'exp': 0}
        levels = {}
        top = max(int(level) for level in rows)
        for level in range(1, top + 1):
            levels[level] = {key: round(value) for key, value in current.items()}
            row = rows.get(str(level)) or table['NONE'].get(str(level))
            if row is None:
                break
            for key, source in FIELDS:
                current[key] += row[source] + (LEVEL_RANDOM if key in ('hp', 'mp') else 0)
            current['exp'] += row['exp']
        result[name] = levels

    lines = ['return {']
    for name in sorted(result):
        lines.append(f'    {lua_key(name)} = {{')
        for level in sorted(result[name]):
            lines.append(f'        [{level}] = {lua_value(result[name][level])},')
        lines.append('    },')
    lines.append('}')
    Path(args.brain).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'growth: {len(result)} classes')


if __name__ == '__main__':
    main()
