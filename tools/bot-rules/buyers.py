import argparse
import json
from pathlib import Path

from lua import lua_key, lua_value

ROOT = Path(__file__).resolve().parents[2]
JSON = ROOT / 'server' / 'game' / 'json'
BRAIN = ROOT / 'scripts' / 'brain' / 'data' / 'buyers.lua'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--brain', default=str(BRAIN))
    args = parser.parse_args()

    items = {item['id']: item for item in json.loads((JSON / 'item.json').read_text(encoding='utf-8')).values()}
    spawned = {spawn['npc'] for spawns in json.loads((JSON / 'npc_spawn.json').read_text(encoding='utf-8')).values()
               for spawn in spawns}
    shops = json.loads((JSON / 'buy.json').read_text(encoding='utf-8'))
    result = {}
    for npc in json.loads((JSON / 'npc.json').read_text(encoding='utf-8')).values():
        if npc['id'] not in spawned or npc.get('buy') is None:
            continue
        for entry in (shops.get(str(npc['buy'])) or {}).values():
            item = items.get(entry['item'])
            if item is None:
                continue
            price = entry['price'] if entry['price'] is not None else item['price']
            found = result.setdefault(item['name'], {'price': price, 'npcs': []})
            found['price'] = max(found['price'], price)
            if npc['name'] not in found['npcs']:
                found['npcs'].append(npc['name'])

    lines = ['return {']
    for name in sorted(result):
        result[name]['npcs'].sort()
        lines.append(f'    {lua_key(name)} = {lua_value(result[name])},')
    lines.append('}')
    Path(args.brain).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'buyers: {len(result)} items')


if __name__ == '__main__':
    main()
