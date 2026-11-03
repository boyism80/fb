import argparse
import json
import math
import random
import re
from pathlib import Path

from lua import lua_value

ROOT = Path(__file__).resolve().parents[2]
JSON = ROOT / 'server' / 'game' / 'json'
DATA = ROOT / 'scripts' / 'brain' / 'data'
SPELLS = ROOT / 'server' / 'bot' / 'config' / 'load-spells.json'
LOGIN_CONFIG = ROOT / 'server' / 'login' / 'config' / 'config.dev.json'
CONFIG = Path(__file__).with_name('snapshots.json')
OUTPUT = ROOT / 'scripts' / 'sim' / 'data'
PROFILES = ROOT / 'scripts' / 'integration' / 'scenarios' / 'brain' / 'state' / 'profiles.lua'
STAT_BASE = 3
CAPACITY = 52
SLOTS = ('WEAPON', 'ARMOR', 'SHIELD', 'HELMET', 'RING', 'RING', 'AUXILIARY', 'AUXILIARY')
NAMES = {'전사': 'WARRIOR', '도적': 'ROGUE', '주술사': 'MAGE', '도사': 'POET'}
NATIONS = (1, 2)


def lua_table(path):
    return path.read_text(encoding='utf-8')


def returns():
    text = lua_table(DATA / 'teleport.lua')
    found = {}
    for nation, body in re.findall(r'\[NATION\.(\w+)\] = \{([^}]*)\}', text):
        found[1 if nation == 'GOGURYEO' else 2] = [int(map) for map in re.findall(r'\d+', body)]
    return found


def shops():
    found = {}
    for name, price in re.findall(r'\["([^"]+)"\] = \{ price = (\d+)', lua_table(DATA / 'shops.lua')):
        found[name] = int(price)
    return found


def effects():
    found = {}
    for name, body in re.findall(r'\["([^"]+)"\] = \{([^}]*)\}', lua_table(DATA / 'items.lua')):
        values = dict((key, float(value)) for key, value in re.findall(r'(\w+) = ([\d.]+)', body))
        found[name] = values
    return found


def mobs():
    found = {}
    for body in re.findall(r'\[\d+\] = \{ id = \d+, (name = .*?), drops = \{(.*?)\}, rezen', lua_table(DATA / 'hunting.lua')):
        head, drops = body
        damage = re.search(r'damage = (\d+)', head)
        found[re.search(r'name = "([^"]+)"', head).group(1)] = {
            'damage': int(damage.group(1)) if damage else 0,
            'drops': set(re.findall(r'name = "([^"]+)"', drops)),
        }
    return found


def floors():
    found = []
    text = lua_table(DATA / 'hunting.lua')
    for mobs_body, exp, rank, supply in re.findall(
            r'\{ map = \d+, name = "[^"]*", mobs = \{(.*?)\}, exp = (\d+), rank = (\d+), supply = ([\d.]+)', text):
        count = sum(int(value) for value in re.findall(r'count = (\d+)', mobs_body))
        if count > 0:
            found.append({'rank': int(rank), 'rate': float(supply) * int(exp) / count})
    return found


def ability_rows():
    table = json.loads((JSON / 'ability.json').read_text(encoding='utf-8'))
    return table


def row(table, name, level):
    return table.get(name, {}).get(str(level)) or table['NONE'].get(str(level))


def top(table, name):
    return max(int(level) for level in table[name])


def grow(table, name, level, init, rng):
    hp = init['hp']['base'] + rng.randint(-init['hp']['range'], init['hp']['range'])
    mp = init['mp']['base'] + rng.randint(-init['mp']['range'], init['mp']['range'])
    stats = {'str': STAT_BASE, 'int': STAT_BASE, 'dex': STAT_BASE}
    exp = 0
    for step in range(1, level):
        current = row(table, name, step)
        if current is None:
            break
        stats['str'] += current['strength']
        stats['int'] += current['intelligence']
        stats['dex'] += current['dexterity']
        hp += current['hp'] + rng.randint(0, 9)
        mp += current['mp'] + rng.randint(0, 9)
        exp += current['exp']
    return {'hp': hp, 'mp': mp, **stats, 'exp': exp}


def mean_hp(table, name, level, init):
    hp = init['hp']['base']
    for step in range(1, level):
        current = row(table, name, step)
        if current is None:
            break
        hp += current['hp'] + 4.5
    return hp


def needed(table, name, level):
    current = row(table, name, level)
    return current['exp'] if current is not None else 0


def weights(table, name, init, grounds):
    result = {}
    for level in range(1, top(table, name) + 1):
        hp = mean_hp(table, name, level, init)
        rate = max((floor['rate'] for floor in grounds if floor['rank'] < hp), default=0)
        gap = needed(table, name, level)
        if rate > 0 and gap > 0:
            result[level] = gap / rate
    return result


def wearable(item, name, level, stats, gender):
    for condition in item.get('condition') or []:
        header, params = condition['Header'], condition['Params']
        if header == 'level' and level < params[0]:
            return False
        elif header == 'level' and len(params) > 1 and params[1] is not None and level > params[1]:
            return False
        elif header == 'strength' and stats['str'] < params[0]:
            return False
        elif header == 'class_t' and name not in params:
            return False
        elif header == 'gender' and gender not in params:
            return False
        elif header == 'promotion':
            return False
    return True


def sold():
    items = json.loads((JSON / 'item.json').read_text(encoding='utf-8'))
    spawned = {spawn['npc'] for spawns in json.loads((JSON / 'npc_spawn.json').read_text(encoding='utf-8')).values()
               for spawn in spawns}
    selling = {str(sell) for npc in json.loads((JSON / 'npc.json').read_text(encoding='utf-8')).values()
               if npc['id'] in spawned for sell in npc.get('sell') or []}
    ids = {entry['item'] for shop, entries in json.loads((JSON / 'sell.json').read_text(encoding='utf-8')).items()
           if shop in selling for entry in entries}
    return {item['name'] for item in items.values() if item['id'] in ids}


def equipment():
    items = json.loads((JSON / 'item.json').read_text(encoding='utf-8'))
    return [item for item in items.values() if item['type'] in set(SLOTS)]


def available(item, store, hp, monsters):
    if item['name'] in store:
        return True
    return any(mob['damage'] < hp and item['name'] in mob['drops'] for mob in monsters.values())


def choose(candidates, bias, rng):
    if not candidates:
        return None
    ordered = sorted(candidates, key=lambda item: (item['price'] or 0, item['name']))
    index = min(len(ordered) - 1, int(len(ordered) * rng.random() ** (1.0 / bias)))
    return ordered[index]


def percent_pool(base, buff, percent):
    total = base + buff
    if total < 1:
        return 1
    if percent == 0:
        return total
    total += int(total * percent / 100.0)
    return max(1, total)


def clamp(value, low, high):
    return max(low, min(high, value))


def derive(stats, gear):
    buff = {'hp': 0, 'mp': 0, 'hp_pct': 0.0, 'mp_pct': 0.0, 'phydef': 0, 'dam': 0, 'hit': 0}
    for item in gear:
        if item is None:
            continue
        buff['hp'] += item.get('base_hp') or 0
        buff['mp'] += item.get('base_mp') or 0
        buff['hp_pct'] += item.get('hp_percentage') or 0.0
        buff['mp_pct'] += item.get('mp_percentage') or 0.0
        buff['phydef'] -= item.get('defensive_physical') or 0
        buff['dam'] += item.get('damage') or 0
        buff['hit'] += item.get('hit') or 0
    return {
        'max_hp': percent_pool(stats['hp'], buff['hp'], buff['hp_pct']),
        'max_mp': percent_pool(stats['mp'], buff['mp'], buff['mp_pct']),
        'phydef': clamp(buff['phydef'], -127, 128),
        'dam': clamp(buff['dam'], -128, 127),
        'hit': clamp(buff['hit'], -128, 127),
    }


def learned(classes, name, level, config, rng):
    beast = rng.randrange(4)
    result = []
    for entry in classes.get(name, {}).get('spells', []):
        if entry['level'] > level:
            continue
        chance = 1.0 - 0.5 ** ((level - entry['level'] + 1) / config['spell_halflife'])
        if rng.random() >= chance:
            continue
        names = entry['names']
        result.append(names[beast] if len(names) == 4 else names[0])
    return [{'slot': index + 1, 'name': spell} for index, spell in enumerate(dict.fromkeys(result))]


def bag(store, potions, config, rng):
    kinds = [name for name in store if name in potions and (potions[name].get('hp') or potions[name].get('mp'))]
    chosen = rng.sample(kinds, min(len(kinds), rng.randint(0, config['potion_kinds'])))
    items = []
    for index, name in enumerate(chosen):
        count = max(1, int(rng.expovariate(1.0 / config['potion_mean'])))
        items.append({'slot': index + 1, 'name': name, 'count': count, 'type': 'CONSUME'})
    return {'items': items, 'used': len(items), 'capacity': CAPACITY}


def position(spawns, maps, rng):
    choices = [(map, spawn['position']) for map in maps for spawn in spawns.get(str(map), [])]
    if not choices:
        return maps[0], 6, 6
    map, point = rng.choice(choices)
    return map, point['x'], point['y'] + 1


def build(index, rng, context, config):
    table, init, grounds, classes, store, potions, monsters, gear_pool, towns, spawns = context
    pick = rng.choice(config['classes'])
    bands = context_weights[pick]
    band = rng.choice(sorted(bands))
    level = rng.choices(list(bands[band]), weights=list(bands[band].values()))[0]
    name = pick if level >= config['job_level'] else 'NONE'
    stats = grow(table, name if name != 'NONE' else 'NONE', level, init, rng)
    gender = 'FEMALE' if rng.random() < config['female'] else 'MALE'
    hp = mean_hp(table, pick, level, init)
    worn = []
    for slot in SLOTS:
        if rng.random() < config['empty'][slot]:
            worn.append(None)
            continue
        key = (name, level, gender, slot)
        if key not in context_candidates:
            context_candidates[key] = [item for item in gear_pool if item['type'] == slot and
                                       wearable(item, name, level, stats, gender) and
                                       available(item, store, hp, monsters)]
        worn.append(choose(context_candidates[key], config['gear_bias'], rng))
    derived = derive(stats, worn)
    full = rng.random() < config['full']
    hp_now = derived['max_hp'] if full else max(1, int(derived['max_hp'] * rng.uniform(0.1, 1.0)))
    mp_now = derived['max_mp'] if full else int(derived['max_mp'] * rng.uniform(0.0, 1.0))
    money = int(rng.lognormvariate(math.log(max(1.0, stats['exp'] * config['money_ratio'])), config['money_sigma']))
    nation = rng.choice(NATIONS)
    map, x, y = position(spawns, towns[nation], rng)
    gap = needed(table, name, level)
    spells = learned(classes, name, level, config, rng) if name != 'NONE' else []
    snap = {
        'self': {
            'name': f'sim{config["seed"]}_{index}',
            'level': level,
            'exp': stats['exp'] + (rng.randint(0, gap - 1) if gap > 0 else 0),
            'class': name,
            'promotion': 0,
            'nation': nation,
            'ghost': False,
            'online': True,
            'hp': hp_now,
            'max_hp': derived['max_hp'],
            'mp': mp_now,
            'max_mp': derived['max_mp'],
            'money': money,
            'phydef': derived['phydef'],
            'dam': derived['dam'],
            'hit': derived['hit'],
            'grouping': rng.random() < config['grouping'],
            'cloaked': False,
        },
        'buffs': [],
        'spells': spells,
        'gear': [{'type': slot, 'names': [item['name']] if item is not None else []} for slot, item in zip(SLOTS, worn)],
        'bag': bag(store, potions, config, rng),
        'world': {'map': map, 'x': x, 'y': y, 'mobs': [], 'items': [], 'dialog': False, 'aside': []},
        'record': {'failures': {}, 'memory': {}},
        'pace': {},
        'now': 0,
        'persona': {},
        'group': [],
        'party': [],
    }
    spec = {'class': pick, 'gender': gender, 'str': stats['str'], 'int': stats['int'], 'dex': stats['dex']}
    return {'spec': spec, 'snap': snap}


def check(table, gear_pool):
    by_name = {item['name']: item for item in gear_pool}
    text = PROFILES.read_text(encoding='utf-8')
    for line in text.splitlines():
        found = re.search(r'name = "(\w+)", class = "([^"]+)", level = (\d+), hp = (\d+), mp = (\d+), str = (\d+)', line)
        if found is None:
            continue
        profile, cls, level, hp, mp, strength = found.groups()
        gear = re.search(r'gear = \{(.*?)\}', line)
        names = re.findall(r'= "([^"]+)"', gear.group(1)) if gear else []
        for gender in ('MALE', 'FEMALE'):
            worn = [by_name[name] for name in names if name in by_name and
                    wearable(by_name[name], NAMES[cls], int(level), {'str': int(strength)}, gender)]
            derived = derive({'hp': int(hp), 'mp': int(mp)}, worn)
            print(f'{profile} {gender} max_hp={derived["max_hp"]} max_mp={derived["max_mp"]} '
                  f'phydef={derived["phydef"]} dam={derived["dam"]} hit={derived["hit"]}')


context_weights = {}
context_candidates = {}


def banded(levels, width):
    bands = {}
    for level, weight in levels.items():
        bands.setdefault((level - 1) // width, {})[level] = weight
    return bands


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default=str(CONFIG))
    parser.add_argument('--output', default=str(OUTPUT))
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()

    config = json.loads(Path(args.config).read_text(encoding='utf-8'))
    table = ability_rows()
    init = json.loads(LOGIN_CONFIG.read_text(encoding='utf-8'))['init']
    gear_pool = equipment()
    if args.check:
        check(table, gear_pool)
        return

    grounds = floors()
    classes = json.loads(SPELLS.read_text(encoding='utf-8'))['classes']
    store = shops()
    potions = effects()
    monsters = mobs()
    store_gear = sold()
    towns = returns()
    spawns = json.loads((JSON / 'npc_spawn.json').read_text(encoding='utf-8'))
    for name in config['classes']:
        context_weights[name] = banded(weights(table, name, init, grounds), config['band'])
    rng = random.Random(config['seed'])
    context = (table, init, grounds, classes, set(store) | store_gear, potions, monsters, gear_pool, towns, spawns)
    records = [build(index, rng, context, config) for index in range(config['count'])]

    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    path = output / f'snapshots_{config["seed"]}.lua'
    lines = ['return {', f'    config = {lua_value(config)},', '    records = {']
    for record in records:
        lines.append(f'        {lua_value(record)},')
    lines.append('    },')
    lines.append('}')
    path.write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print(f'snapshots: {len(records)} -> {path}')


if __name__ == '__main__':
    main()
