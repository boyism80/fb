import argparse
import json
from pathlib import Path

from lua import lua_key, lua_value

ROOT = Path(__file__).resolve().parents[2]
JSON = ROOT / 'server' / 'game' / 'json'
ITEMS = ROOT / 'server' / 'bot' / 'config' / 'load-items.json'
BRAIN = ROOT / 'scripts' / 'brain' / 'data' / 'shops.lua'
OUTFITS = ROOT / 'scripts' / 'brain' / 'data' / 'outfits.lua'
WEARABLE = ('WEAPON', 'ARMOR', 'SHIELD', 'HELMET', 'RING', 'AUXILIARY')
RECOVERY = ('hp', 'mp', 'hp_ratio', 'mp_ratio', 'hp_full', 'mp_full')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--brain', default=str(BRAIN))
    parser.add_argument('--outfits', default=str(OUTFITS))
    args = parser.parse_args()

    items = {item['id']: item for item in json.loads((JSON / 'item.json').read_text(encoding='utf-8')).values()}
    effects = json.loads(ITEMS.read_text(encoding='utf-8'))
    spawned = {spawn['npc'] for spawns in json.loads((JSON / 'npc_spawn.json').read_text(encoding='utf-8')).values()
               for spawn in spawns}
    sells = json.loads((JSON / 'sell.json').read_text(encoding='utf-8'))
    result = {}
    outfits = {}
    for npc in json.loads((JSON / 'npc.json').read_text(encoding='utf-8')).values():
        if npc['id'] not in spawned:
            continue
        for sell in npc.get('sell') or []:
            for entry in sells.get(str(sell)) or []:
                item = items.get(entry['item'])
                if item is not None and item['type'] in WEARABLE:
                    outfit(outfits, item, entry, npc)
                    continue
                if item is None or not any(key in effects.get(item['name'], {}) for key in RECOVERY):
                    continue
                price = entry['price'] if entry['price'] is not None else item['price']
                uses = (item.get('durability') or 1) if item['type'] == 'PACKAGE' else 1
                found = result.setdefault(item['name'], {'price': price, 'capacity': item['capacity'], 'uses': uses,
                                                         'npcs': []})
                found['price'] = min(found['price'], price)
                if npc['name'] not in found['npcs']:
                    found['npcs'].append(npc['name'])

    write(args.brain, result)
    write(args.outfits, outfits)
    print(f'shops: {len(result)} items, outfits: {len(outfits)} items')


def outfit(outfits, item, entry, npc):
    price = entry['price'] if entry['price'] is not None else item['price']
    found = outfits.get(item['name'])
    if found is None:
        found = {'type': item['type'], 'price': price, 'npcs': []}
        for condition in item.get('condition') or []:
            header, params = condition['Header'], condition['Params']
            if header == 'level':
                found['level'] = params[0]
                if len(params) > 1 and params[1] is not None:
                    found['limit'] = params[1]
            elif header == 'class_t':
                found['class'] = params[0]
            elif header in ('gender', 'strength', 'promotion'):
                found[header] = params[0]
        outfits[item['name']] = found
    found['price'] = min(found['price'], price)
    if npc['name'] not in found['npcs']:
        found['npcs'].append(npc['name'])


def write(path, entries):
    lines = ['return {']
    for name in sorted(entries):
        entries[name]['npcs'].sort()
        lines.append(f'    {lua_key(name)} = {lua_value(entries[name])},')
    lines.append('}')
    Path(path).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')


if __name__ == '__main__':
    main()
