import argparse
import json
import re
from pathlib import Path

from lua import lua_key, lua_value

ROOT = Path(__file__).resolve().parents[2]
JSON = ROOT / 'server' / 'game' / 'json'
SPELLS = ROOT / 'server' / 'bot' / 'config' / 'load-spells.json'
GROWTH = ROOT / 'scripts' / 'brain' / 'data' / 'growth.lua'
OUTPUT = ROOT / 'scripts' / 'integration' / 'scenarios' / 'brain' / 'state' / 'profiles.lua'
CLASSES = {'WARRIOR': '전사', 'ROGUE': '도적', 'MAGE': '주술사', 'POET': '도사'}
STAGES = (('MAGE', 6), ('POET', 18), ('ROGUE', 50), ('WARRIOR', 63), ('WARRIOR', 10), ('ROGUE', 20), ('MAGE', 35),
          ('POET', 50))
SLOTS = {'WEAPON': lambda item: sum(item['damage_small'].values()), 'ARMOR': lambda item: item['defensive_physical'],
         'HELMET': lambda item: item['defensive_physical']}


def growth(name, level):
    text = GROWTH.read_text(encoding='utf-8')
    block = text[text.index(f'    {name} = {{'):]
    found = re.search(rf'\[{level}\] = \{{ hp = (\d+), mp = (\d+), str = (\d+), int = (\d+), dex = (\d+)', block)
    return dict(zip(('hp', 'mp', 'str', 'int', 'dex'), map(int, found.groups())))


def wearable(item, name, level, stats):
    for condition in item.get('condition') or []:
        header, params = condition['Header'], condition['Params']
        if header == 'level' and level < params[0]:
            return False
        elif header == 'strength' and stats['str'] < params[0]:
            return False
        elif header == 'class_t' and name not in params:
            return False
        elif header == 'gender' and 'MALE' not in params:
            return False
        elif header == 'promotion':
            return False
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    args = parser.parse_args()

    items = {item['id']: item for item in json.loads((JSON / 'item.json').read_text(encoding='utf-8')).values()}
    spawned = {spawn['npc'] for spawns in json.loads((JSON / 'npc_spawn.json').read_text(encoding='utf-8')).values()
               for spawn in spawns}
    shops = {str(sell) for npc in json.loads((JSON / 'npc.json').read_text(encoding='utf-8')).values()
             if npc['id'] in spawned for sell in npc.get('sell') or []}
    sold = {entry['item'] for shop, entries in json.loads((JSON / 'sell.json').read_text(encoding='utf-8')).items()
            if shop in shops for entry in entries}
    ability = json.loads((JSON / 'ability.json').read_text(encoding='utf-8'))
    classes = json.loads(SPELLS.read_text(encoding='utf-8'))['classes']
    lines = ['return {']
    for name, level in STAGES:
        stats = growth(name, level)
        budget = sum((ability[name].get(str(step)) or ability['NONE'].get(str(step)) or {'exp': 0})['exp']
                     for step in range(1, level))
        spells = [entry['names'][0] for entry in classes[name]['spells'] if 0 < entry['level'] <= level]
        gear = {}
        for slot, value in SLOTS.items():
            choices = [items[id] for id in sold if id in items and items[id]['type'] == slot and
                       items[id]['price'] <= budget and wearable(items[id], name, level, stats)]
            if choices:
                gear[slot] = max(choices, key=value)['name']
        profile = {'name': f'{name.lower()}{level}', 'class': CLASSES[name], 'level': level, **stats,
                   'spells': spells, 'gear': gear}
        lines.append(f'    {lua_value(profile)},')
    lines.append('}')
    Path(args.output).write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'profiles: {len(STAGES)}')


if __name__ == '__main__':
    main()
