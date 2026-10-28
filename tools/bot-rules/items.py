import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / 'server' / 'game' / 'scripts'
ITEM_JSON = ROOT / 'server' / 'game' / 'json' / 'item.json'
OUTPUT = ROOT / 'server' / 'bot' / 'config' / 'load-items.json'

ACTIVATED = re.compile(r"^(\s*)on_activated\s*=\s*function\(.*?\)\n(.*?)^\1end,?\s*$", re.M | re.S)
HP_UP = re.compile(r"me:hp\(math\.min\(me:hp\(\)\s*\+\s*(\d+)")
HP_DOWN = re.compile(r"me:hp\(math\.max\(\d+,\s*me:hp\(\)\s*-\s*(\d+)")
MP_UP = re.compile(r"me:mp\(math\.min\(me:mp\(\)\s*\+\s*(\d+)")
HP_FULL = re.compile(r"me:hp\(me:maxhp\(\)")
MP_FULL = re.compile(r"me:mp\(me:maxmp\(\)")
HP_RATIO = re.compile(r"local\s+(\w+)\s*=\s*me:maxhp\(\)\s*//\s*(\d+)")
MP_RATIO = re.compile(r"local\s+(\w+)\s*=\s*me:maxmp\(\)\s*//\s*(\d+)")
MAP_NAME = re.compile(r"me:map\('([^']+)'")
MAP_TABLE = re.compile(r"local\s+maps\s*=\s*\{([^}]*)\}")


def effect(body):
    info = {}
    hp = sum(int(value) for value in HP_UP.findall(body)) - sum(int(value) for value in HP_DOWN.findall(body))
    mp = sum(int(value) for value in MP_UP.findall(body))
    if hp != 0:
        info['hp'] = hp
    if mp != 0:
        info['mp'] = mp
    if HP_FULL.search(body):
        info['hp_full'] = True
    if MP_FULL.search(body):
        info['mp_full'] = True
    for name, divisor in HP_RATIO.findall(body):
        if f'me:hp() + {name}' in body:
            info['hp_ratio'] = 1 / int(divisor)
    for name, divisor in MP_RATIO.findall(body):
        if f'me:mp() + {name}' in body:
            info['mp_ratio'] = 1 / int(divisor)
    maps = MAP_NAME.findall(body)
    table = MAP_TABLE.search(body)
    if table is not None and 'me:map(map' in body:
        maps += re.findall(r"'([^']+)'", table.group(1))
    if 'warp_to_return_map' in body:
        info['warp'] = 'RETURN'
    elif maps:
        info['warp'] = 'MAP'
        info['maps'] = maps
    elif ':warp(' in body or 'warp_to' in body:
        info['warp'] = 'OTHER'
    if ':cast(' in body or ':buff(' in body:
        info['buff'] = True
    if 'base_hp' in body or 'base_mp' in body:
        info['stat'] = True
    if ':quest(' in body or 'quest.' in body:
        info['quest'] = True
    if not info:
        info['unknown'] = True
    return info


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    args = parser.parse_args()

    table = json.loads(ITEM_JSON.read_text(encoding='utf-8'))
    result = {}
    for item in table.values():
        path = SCRIPTS / 'item' / f"{item['id']}.lua"
        if not path.exists():
            continue

        found = ACTIVATED.search(path.read_text(encoding='utf-8'))
        if found is None:
            continue

        info = effect(found.group(2))
        info['id'] = item['id']
        info['type'] = item['type']
        result[item['name']] = info

    Path(args.output).write_text(json.dumps(result, ensure_ascii=False, indent=1), encoding='utf-8')
    kinds = {}
    for info in result.values():
        for key in ('hp', 'mp', 'hp_ratio', 'mp_ratio', 'hp_full', 'mp_full', 'warp', 'buff', 'stat', 'quest', 'unknown'):
            if key in info:
                kinds[key] = kinds.get(key, 0) + 1
    print(f'items={len(result)} {kinds}')


if __name__ == '__main__':
    main()
