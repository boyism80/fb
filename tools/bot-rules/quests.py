import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / 'server' / 'game' / 'scripts'
OUTPUT = ROOT / 'server' / 'bot' / 'config' / 'load-quests.json'

CONSTANT = re.compile(r"^M\.(QUEST_\w+)\s*=\s*(\d+)", re.M)
REFERENCE = re.compile(r"quest\.(QUEST_\w+)")
START = re.compile(r"start_quest\(\s*quest\.(QUEST_\w+)")
LOCAL = re.compile(r"local\s+(\w+)\s*=\s*['\"]([^'\"]+)['\"]")
NAME = r"(?:'([^']+)'|\"([^\"]+)\"|([A-Z_][A-Z0-9_]*))"
HAS = re.compile(r"has_items\(\s*" + NAME + r"\s*,\s*(\d+)")
REMOVE = re.compile(r"rmitem\(\s*" + NAME + r"\s*,\s*(\d+)")
MAKE = re.compile(r"mkitem\(\s*" + NAME + r"(?:\s*,\s*(\d+))?")
ENTRY = re.compile(r"\[\s*" + NAME + r"\s*\]\s*=\s*(\d+)")
MONEY_KEY = re.compile(r"\['money'\]\s*=\s*(\d+|[A-Z_][A-Z0-9_]*)")
EXP_KEY = re.compile(r"\['exp'\]\s*=\s*(\d+|[A-Z_][A-Z0-9_]*)")
NUMBER = re.compile(r"local\s+([A-Z_][A-Z0-9_]*)\s*=\s*(\d+)\b")
EXP_CALL = re.compile(r"me:exp\(\s*me:exp\(\)\s*\+\s*(\d+)")
MONEY_CALL = re.compile(r"me:money\(\s*me:money\(\)\s*([+-])\s*(\d+)")
LEVEL = re.compile(r"me:level\(\)\s*(<=|<|>=|>)\s*(\d+)")
STEP = re.compile(r":step\((\d+)\)")
BRANCH = re.compile(r"step\s*==\s*(\d+)")
OPTIONS = re.compile(r"me:list\(\s*npc\s*,[^{]*\{([^}]*)\}")
COMPLETE = re.compile(r":complete\(\)")


def constants():
    text = (SCRIPTS / 'lib' / 'quest.lua').read_text(encoding='utf-8')
    return {name: int(value) for name, value in CONSTANT.findall(text)}


def resolve(match, names, offset=0):
    literal = match.group(offset + 1) or match.group(offset + 2)
    if literal:
        return literal
    return names.get(match.group(offset + 3))


def block(text, start):
    depth = 0
    for index in range(start, len(text)):
        if text[index] == '{':
            depth += 1
        elif text[index] == '}':
            depth -= 1
            if depth == 0:
                return text[start:index + 1], index + 1
    return text[start:], len(text)


def tables(text, call):
    result = []
    for found in re.finditer(re.escape(call) + r"\(", text):
        cursor = found.end()
        parts = []
        while len(parts) < 2:
            brace = text.find('{', cursor)
            close = text.find(')', cursor)
            if brace < 0 or (0 <= close < brace):
                break
            part, cursor = block(text, brace)
            parts.append(part)
        result.append(parts)
    return result


def table_items(part, names):
    items = {}
    inner = re.search(r"\['item'\]\s*=\s*\{", part)
    if inner:
        body, _ = block(part, inner.end() - 1)
        for entry in ENTRY.finditer(body):
            name = resolve(entry, names)
            if name:
                items[name] = items.get(name, 0) + int(entry.group(4))
    money = MONEY_KEY.search(part)
    exp = EXP_KEY.search(part)
    return items, number(money, names), number(exp, names)


def number(found, names):
    if found is None:
        return 0
    value = found.group(1)
    value = value if value.isdigit() else names.get(value, '0')
    return int(value) if value.isdigit() else 0


def segments(text, known):
    marks = [(found.start(), found.group(1)) for found in REFERENCE.finditer(text) if found.group(1) in known]
    result = {}
    for index, (position, name) in enumerate(marks):
        end = len(text)
        for later, other in marks[index + 1:]:
            if other != name:
                end = later
                break
        result.setdefault(name, []).append(text[position:end])
    return result


def merge(target, items):
    for name, count in items.items():
        target[name] = max(target.get(name, 0), count)


def gate(entry, text):
    for operator, value in LEVEL.findall(text):
        level = int(value) if operator in ('<', '>=') else int(value) + 1
        entry['level'] = max(entry['level'], level)


def choices(entry, text):
    for options in OPTIONS.findall(text):
        for option in re.findall(r"['\"]([^'\"]+)['\"]", options):
            if option not in entry['picks']:
                entry['picks'].append(option)


def transforms():
    edges = {}
    for path in sorted((SCRIPTS / 'item').glob('*.lua')):
        text = path.read_text(encoding='utf-8')
        names = dict(LOCAL.findall(text))
        for parts in tables(text, 'exchange'):
            if len(parts) != 2:
                continue
            cost, _, _ = table_items(parts[0], names)
            gain, _, _ = table_items(parts[1], names)
            for source in cost:
                edges.setdefault(source, set()).update(gain)
    return edges


def derived(items, edges):
    found = set()
    pending = list(items)
    while pending:
        item = pending.pop()
        for target in edges.get(item, ()):
            if target not in found:
                found.add(target)
                pending.append(target)
    return found


def analyze(entry, text, names, made):
    for found in HAS.finditer(text):
        name = resolve(found, names)
        if name:
            merge(entry['requires'], {name: int(found.group(4))})
    for found in REMOVE.finditer(text):
        name = resolve(found, names)
        if name:
            merge(entry['consumes'], {name: int(found.group(4))})
    for found in MAKE.finditer(text):
        name = resolve(found, names)
        if name:
            merge(entry['rewards']['items'], {name: int(found.group(4) or 1)})
    for parts in tables(text, 'exchange'):
        if len(parts) == 2:
            cost, money, _ = table_items(parts[0], names)
            gain, earned, exp = table_items(parts[1], names)
            if not cost and gain and set(gain) <= made and earned == 0 and exp == 0:
                continue
            merge(entry['consumes'], cost)
            merge(entry['rewards']['items'], gain)
            entry['money'] = max(entry['money'], money)
            entry['rewards']['money'] = max(entry['rewards']['money'], earned)
            entry['rewards']['exp'] = max(entry['rewards']['exp'], exp)
    for parts in tables(text, 'consume'):
        if parts:
            cost, money, _ = table_items(parts[0], names)
            merge(entry['consumes'], cost)
            entry['money'] = max(entry['money'], money)
    for parts in tables(text, 'reward'):
        if parts:
            gain, earned, exp = table_items(parts[0], names)
            merge(entry['rewards']['items'], gain)
            entry['rewards']['money'] = max(entry['rewards']['money'], earned)
            entry['rewards']['exp'] = max(entry['rewards']['exp'], exp)
    for found in EXP_CALL.finditer(text):
        entry['rewards']['exp'] = max(entry['rewards']['exp'], int(found.group(1)))
    for sign, amount in MONEY_CALL.findall(text):
        if sign == '+':
            entry['rewards']['money'] = max(entry['rewards']['money'], int(amount))
        else:
            entry['money'] = max(entry['money'], int(amount))
    gate(entry, text)
    entry['steps'].update(int(value) for value in STEP.findall(text))
    entry['steps'].update(int(value) for value in BRANCH.findall(text))
    choices(entry, text)
    if COMPLETE.search(text):
        entry['completes'] = True


def build():
    known = constants()
    quests = {}
    for name, number in known.items():
        quests[name] = {
            'id': number, 'name': name, 'npcs': [], 'starts': [], 'mobs': [], 'items': [], 'maps': [],
            'level': 0, 'requires': {}, 'consumes': {}, 'money': 0, 'steps': set(), 'picks': [],
            'completes': False, 'rewards': {'items': {}, 'money': 0, 'exp': 0},
        }

    for kind, key in (('npc', 'npcs'), ('mob', 'mobs'), ('item', 'items'), ('map', 'maps')):
        for path in sorted((SCRIPTS / kind).glob('*.lua')):
            if not path.stem.isdigit():
                continue
            text = path.read_text(encoding='utf-8')
            if 'quest.QUEST_' not in text:
                continue
            names = dict(LOCAL.findall(text))
            names.update(NUMBER.findall(text))
            for start in START.findall(text):
                if start in quests and int(path.stem) not in quests[start]['starts']:
                    quests[start]['starts'].append(int(path.stem))
            marks = [found.start() for found in REFERENCE.finditer(text) if found.group(1) in known]
            head = text[:marks[0]] if marks else ''
            for name, parts in segments(text, known).items():
                entry = quests[name]
                if int(path.stem) not in entry[key]:
                    entry[key].append(int(path.stem))
                if kind == 'npc':
                    gate(entry, head)
                    choices(entry, head)
                    made = {resolve(found, names) for part in parts for found in MAKE.finditer(part)}
                    for part in parts:
                        analyze(entry, part, names, made)

    edges = transforms()
    result = []
    for entry in sorted(quests.values(), key=lambda value: value['id']):
        if not (entry['npcs'] or entry['mobs'] or entry['items'] or entry['maps']):
            continue
        entry['steps'] = sorted(entry['steps'])
        inner = derived(set(entry['rewards']['items']) - set(entry['consumes']), edges)
        entry['consumes'] = {name: count for name, count in entry['consumes'].items() if name not in inner}
        entry['requires'] = {name: count for name, count in entry['requires'].items() if name not in inner}
        entry['rewards']['items'] = {
            name: count for name, count in entry['rewards']['items'].items() if name not in entry['consumes']
        }
        result.append(entry)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    args = parser.parse_args()

    quests = build()
    rewarded = {}
    for entry in quests:
        for item in entry['rewards']['items']:
            rewarded.setdefault(item, []).append(entry['id'])

    Path(args.output).write_text(json.dumps({'quests': quests, 'rewards': rewarded}, ensure_ascii=False, indent=1), encoding='utf-8')
    started = sum(1 for entry in quests if entry['starts'])
    with_items = sum(1 for entry in quests if entry['rewards']['items'])
    print(f'quests={len(quests)} started={started} item_rewards={with_items} reward_items={len(rewarded)}')


if __name__ == '__main__':
    main()
