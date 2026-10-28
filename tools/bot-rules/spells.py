import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / 'server' / 'game' / 'scripts'
SPELL_JSON = ROOT / 'server' / 'game' / 'json' / 'spell.json'
OUTPUT = ROOT / 'server' / 'bot' / 'config' / 'load-spells.json'

KEY = re.compile(r"^\s*\[(?:'(?P<name>[^']+)'|spell\.(?P<helper>divine_beast_\w*spell)\(me:divine_beast\(\),\s*(?P<beast>\d+)\))\]\s*=\s*\{\s*$")
LEVEL = re.compile(r"\['level'\]\s*=\s*(\d+)")
MATERIAL = re.compile(r"\['name'\]\s*=\s*'([^']+)',\s*\['count'\]\s*=\s*(\d+)")
GUILD = re.compile(r"npc\.basic_class\(me,\s*npc_obj,\s*CLASS\.(\w+),\s*spells\)")
LOCAL = re.compile(r"local\s+(\w+)\s*=\s*(\d+)")
BEAST = re.compile(r"index == (\d+) then\s*return (?:'([^']+)'|\"([^\"]+)\")")
CURE = re.compile(r"you:unbuff\('([^']+)'\)")
CAST = re.compile(r"^\s*on_cast\s*=\s*function", re.M)
BOOST = re.compile(r"me:(\w+)\(me:\1\(\)\s*\+\s*(\d+)\)")


def divine_beast_names():
    text = (SCRIPTS / 'lib' / 'spell.lua').read_text(encoding='utf-8')
    names = {}
    for helper in re.findall(r"function M\.(divine_beast_\w*spell)\(", text):
        body = text.split(f'function M.{helper}(', 1)[1].split('function M.', 1)[0]
        for index, single, double in BEAST.findall(body):
            names.setdefault(helper, {}).setdefault(int(index), []).append(single or double)
    return names


def guilds(beasts):
    result = {}
    for path in sorted((SCRIPTS / 'npc').glob('*.lua')):
        text = path.read_text(encoding='utf-8')
        found = GUILD.search(text)
        if found is None:
            continue

        spells = []
        current = None
        for line in text.split('local spells = {', 1)[1].splitlines()[1:]:
            if line.rstrip() == '        }':
                break

            key = KEY.match(line)
            if key:
                if key.group('name'):
                    current = {'names': [key.group('name')]}
                else:
                    current = {'names': beasts[key.group('helper')][int(key.group('beast'))]}
                spells.append(current)
                continue

            if current is None:
                continue

            level = LEVEL.search(line)
            if level:
                current['level'] = int(level.group(1))

            materials = MATERIAL.findall(line)
            if materials:
                current['money'] = sum(int(count) for name, count in materials if name == '금전')
                current['materials'] = [{'name': name, 'count': int(count)} for name, count in materials if name != '금전']

        result[found.group(1)] = {'npc': int(path.stem), 'spells': sorted(spells, key=lambda x: x['level'])}
    return result


def effect(text):
    values = {key: int(value) for key, value in LOCAL.findall(text)}
    info = {'mp': values.get('mp', 0)}
    cure = CURE.search(text)
    if CAST.search(text) is None:
        info['role'] = 'NONE'
    elif 'spell.heal(' in text:
        info['role'] = 'HEAL'
        info['hp'] = values.get('hp', 0)
        info['self'] = 'spell.heal(me, me' in text
    elif 'spell.damage_near_target' in text:
        info['role'] = 'AREA'
        info['damage'] = values.get('damage', 0)
    elif 'spell.damage(' in text:
        info['role'] = 'ATTACK'
        info['damage'] = values.get('damage', 0)
    elif 'spell.attack_cast' in text:
        info['role'] = 'STRIKE'
        cost = re.search(r"local\s+hp\s*=\s*me:hp\(\)\s*(?:\*\s*(\d+)\s*)?(?://\s*(\d+)|-\s*(\d+))?", text)
        if cost is not None:
            if cost.group(3):
                info['hp_cost'] = 1.0
            else:
                info['hp_cost'] = int(cost.group(1) or 1) / int(cost.group(2) or 1)
        scale = re.search(r"local\s+damage\s*=\s*me:hp\(\)\s*(?:\*\s*(\d+))?", text)
        if scale is not None:
            info['damage_hp'] = int(scale.group(1) or 1)
    elif 'spell.weapon_damage' in text:
        info['role'] = 'ENCHANT'
    elif 'spell.debuff_cast' in text:
        info['role'] = 'DEBUFF'
        info['buff_time'] = values.get('buff_time', 0)
    elif cure is not None:
        info['role'] = 'CURE'
        info['cure'] = cure.group(1)
    elif 'you:buffs' in text:
        info['role'] = 'DISPEL'
    elif 'spell.disguise' in text:
        info['role'] = 'DISGUISE'
    elif 'you:title' in text:
        info['role'] = 'INSPECT'
    elif 'me:attack(' in text:
        info['role'] = 'AMBUSH'
    elif 'oblivion' in text:
        info['role'] = 'ESCAPE'
    elif 'you:target(me)' in text:
        info['role'] = 'TAUNT'
    elif 'me:gain(' in text:
        info['role'] = 'LOOT'
    elif 'spell.teleport_lookup' in text:
        info['role'] = 'GOTO' if 'ch:position' in text else 'CALL'
    elif 'me:spawn_mob' in text:
        info['role'] = 'SUMMON'
    elif 'you:mp(' in text:
        info['role'] = 'TRANSFER'
    elif 'you:state' in text:
        info['role'] = 'REVIVE'
    elif 'me:buff(' in text or ':buff(sp' in text:
        info['role'] = 'BUFF'
        info['buff_time'] = values.get('buff_time', 0)
        info['self'] = 'buff_cast(me, me' in text
        info['exclusive'] = 'me:unbuff(' in text
        if 'me:invincible' in text:
            info['trigger'] = 'EMERGENCY'
        elif 'me:mp_up' in text:
            info['trigger'] = 'MANA'
        boosts = {stat: int(value) for stat, value in BOOST.findall(text)}
        if boosts:
            info['boost'] = boosts
    elif 'spell.boolean_random' in text and 'me:maxmp' in text:
        info['role'] = 'CHARGE'
    elif 'root:cardinal' in text or 'root:revive' in text or 'me:mp_down' in text:
        info['role'] = 'TRAVEL'
    elif 'broadcast(' in text:
        info['role'] = 'SHOUT'
    else:
        info['role'] = 'OTHER'
    return info


def spells(names):
    table = json.loads(SPELL_JSON.read_text(encoding='utf-8'))
    result = {}
    for spell in table.values():
        if spell['name'] not in names:
            continue

        path = SCRIPTS / 'spell' / f"{spell['id']}.lua"
        info = effect(path.read_text(encoding='utf-8')) if path.exists() else {'role': 'OTHER', 'mp': 0}
        info['id'] = spell['id']
        info['type'] = spell['type']
        result[spell['name']] = info
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    args = parser.parse_args()

    beasts = divine_beast_names()
    classes = guilds(beasts)
    names = {name for guild in classes.values() for spell in guild['spells'] for name in spell['names']}
    catalog = {'classes': classes, 'spells': spells(names)}
    Path(args.output).write_text(json.dumps(catalog, ensure_ascii=False, indent=1), encoding='utf-8')
    counts = {name: len(guild['spells']) for name, guild in classes.items()}
    print(f'classes={counts} spells={len(catalog["spells"])}')


if __name__ == '__main__':
    main()
