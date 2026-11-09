import argparse
import json
import re
import struct
from pathlib import Path

from lua import lua_value

ROOT = Path(__file__).resolve().parents[2]
JSON = ROOT / 'server' / 'game' / 'json'
MAPS = ROOT / 'server' / 'game' / 'maps'
OUTPUT = ROOT / 'scripts' / 'brain' / 'data' / 'hunting.lua'
HITTING = {'AGGRESSIVE', 'RETALIATE', 'STATIONARY'}
SERIES = re.compile(r'^(.*?)(\d+)(\D*)$')
HUNTING_GROUND = 0x10
ENABLE_PK = 0x20


def milliseconds(value):
    if not value:
        return 0
    hours, minutes, seconds = value.split(':')
    if '.' in hours:
        days, hours = hours.split('.')
        hours = int(days) * 24 + int(hours)
    return int(round((int(hours) * 3600 + int(minutes) * 60 + float(seconds)) * 1000))


def hunting(option):
    if isinstance(option, str):
        return option == 'HUNTING_GROUND'
    flags = int(option or 0)
    return flags & HUNTING_GROUND != 0 and flags & ENABLE_PK == 0


def map_area(key):
    path = MAPS / f'{int(key):06}.map'
    if path.exists() is False:
        return 0
    with path.open('rb') as file:
        header = file.read(4)
    if len(header) < 4:
        return 0
    width, height = struct.unpack('>HH', header)
    return width * height


def floor(key, rows, mobs, maps):
    counts = {}
    supply = 0.0
    for row in rows:
        mob = mobs.get(str(row['mob']))
        if mob is None or mob.get('invincible') or row.get('count', 0) <= 0:
            continue
        counts[row['mob']] = counts.get(row['mob'], 0) + row['count']
        rezen = milliseconds(row.get('rezen'))
        if rezen > 0:
            supply += row['count'] * 1000.0 / rezen
    if not counts:
        return None

    area = map_area(key)
    hitting = [mobs[str(mob)] for mob in counts if mobs[str(mob)].get('attack_type') in HITTING]
    return {
        'map': int(key),
        'name': maps[key]['name'],
        'mobs': [{'id': mob, 'count': count} for mob, count in sorted(counts.items())],
        'exp': sum(mobs[str(mob)].get('exp', 0) * count for mob, count in counts.items()),
        'rank': max((mob['damage']['max'] for mob in hitting), default=0),
        'supply': round(supply, 4),
        'density': round(sum(counts.values()) / area, 4) if area > 0 else 0,
    }


def loot(drop, items):
    slots = 0.0
    for entry in drop.get('dsl', []):
        params = entry.get('Params') or []
        if entry.get('Header') != 'item' or len(params) < 5 or params[0] not in items:
            continue
        chance = (params[4] or 0) / 100
        count = params[1] or 1
        slots += chance * count / max(1, items[params[0]].get('capacity') or 1)
    return round(slots, 6)


def drops(drop, items):
    found = []
    for entry in drop.get('dsl', []):
        params = entry.get('Params') or []
        if entry.get('Header') != 'item' or len(params) < 5 or params[0] not in items:
            continue
        found.append({'name': items[params[0]]['name'], 'count': params[1] or 1,
                      'chance': round((params[4] or 0) / 100, 6),
                      'capacity': max(1, items[params[0]].get('capacity') or 1)})
    return found


def bundles():
    mobs = json.loads((JSON / 'mob.json').read_text(encoding='utf-8'))
    maps = json.loads((JSON / 'map.json').read_text(encoding='utf-8'))
    spawns = json.loads((JSON / 'mob_spawn.json').read_text(encoding='utf-8'))
    series = {}
    for key, rows in spawns.items():
        if key not in maps or hunting(maps[key].get('option')) is False:
            continue
        found = floor(key, rows, mobs, maps)
        if found is None:
            continue
        match = SERIES.match(found['name'])
        name = (match.group(1) + match.group(3)) if match else found['name']
        found['number'] = int(match.group(2)) if match else 0
        series.setdefault(name, []).append(found)

    result = []
    for name, floors in series.items():
        floors.sort(key=lambda found: (found['number'], found['map']))
        result.append({'name': name, 'floors': floors, 'rank': floors[0]['rank']})
    result.sort(key=lambda bundle: (bundle['rank'], bundle['floors'][0]['map']))

    used = sorted({mob['id'] for bundle in result for found in bundle['floors'] for mob in found['mobs']})
    table_drops = json.loads((JSON / 'drop.json').read_text(encoding='utf-8'))
    items = {item['id']: item for item in json.loads((JSON / 'item.json').read_text(encoding='utf-8')).values()}
    rezens = {}
    for rows in spawns.values():
        for row in rows:
            rezen = milliseconds(row.get('rezen'))
            if rezen > 0:
                rezens[row['mob']] = min(rezens.get(row['mob'], rezen), rezen)
    table = {}
    for id in used:
        mob = mobs[str(id)]
        table[id] = {
            'id': id,
            'name': mob['name'],
            'hp': mob.get('hp', 0),
            'exp': mob.get('exp', 0),
            'damage': mob['damage']['max'],
            'least': mob['damage']['min'],
            'speed': milliseconds(mob.get('speed')),
            'defense': mob.get('defensive_physical', 0),
            'ward': mob.get('defensive_magical', 0),
            'attack': mob.get('attack_type', 'NONE'),
            'size': mob.get('size', 'SMALL'),
            'resists': {key: value for key, value in (mob.get('resist') or {}).items()},
            'loot': loot(table_drops.get(mob.get('drop') or '', {}), items),
            'drops': drops(table_drops.get(mob.get('drop') or '', {}), items),
            'rezen': rezens.get(id, 0),
        }
    return result, table


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    args = parser.parse_args()

    found, table = bundles()
    lines = ['return {', '    bundles = {']
    for bundle in found:
        lines.append(f'        {lua_value(bundle)},')
    lines.append('    },')
    lines.append('    mobs    = {')
    for id in sorted(table):
        lines.append(f'        [{id}] = {lua_value(table[id])},')
    lines.append('    },')
    lines.append('}')
    Path(args.output).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'bundles={len(found)} floors={sum(len(bundle["floors"]) for bundle in found)}')


if __name__ == '__main__':
    main()
