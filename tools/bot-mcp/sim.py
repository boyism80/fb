import argparse
import collections
import glob
import json
import os
import re
import sys
import time
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
MASTER = os.environ.get('BOT_MASTER', 'http://127.0.0.1:8089')
PLAN = os.path.join(ROOT, 'scripts', 'sim', 'world', 'lane_plan.lua')
BOT = os.path.join(ROOT, 'server', 'bot')
LOGS = os.path.join(BOT, 'logs', 'cluster')
TARGET = '175.123.108.99:30000'
ANSI = re.compile(r'\x1b\[[0-9;]*m')
TEAM = re.compile(r'\[sim:team\] (.+?) seed=(\d+) (\w+) lv(\d+) died=(\w+).*?kills=(\d+) exp=(\d+)')
SOLO = re.compile(r'\[sim:world\] (\w+) lv(\d+) ground=\S+ map=\d+ time=\d+s died=(\w+) left=\w+ kills=(\d+) exp=(\d+)')
OUTPUT = re.compile(r'output=(\S+?\.jsonl)')
BATTLE = re.compile(r'\[brain:battle\] bot=(\S+) hp=(\d+) mp=(\d+) mob=(\S*) (.*) picks (\S+)')
FAILED = re.compile(r'failed: .*|lua error.*')
ALL_TEAMS = ['WA+PO', 'RO+PO', 'WA9+PO', 'WA63+PO65', 'WA+PO 세작', 'RO+PO 세작', 'WA+PO 흉가', 'RO+PO 흉가',
             'profile 63+65', 'MA', 'PO', 'RO', 'WA']
DRINKS = ('동동주', '막걸리', '백세주', '오십세주', '팔십세주', '홍주', '파란시약')
STRIKES = ('건곤대나이', '필살검무')
WATCHED = ('보호', '무장', '공력증강', '건곤대나이', '필살검무', '투명', '비영승보', '혼마술', '마비', '절망', '중독')


def call(path, body=None):
    data = json.dumps(body).encode() if body is not None else None
    request = urllib.request.Request(MASTER + path, data=data, headers={'Content-Type': 'application/json'})
    return json.loads(urllib.request.urlopen(request, timeout=30).read())


def write_plan(teams, seeds, solo, lanes, minutes, profile=False, trace=False):
    names = ', '.join(json.dumps(name, ensure_ascii=False) for name in teams)
    numbers = ', '.join(str(seed) for seed in seeds)
    with open(PLAN, 'w', encoding='utf-8', newline='\n') as plan:
        plan.write(f'return {{ lanes = {lanes}, seeds = {{ {numbers} }}, teams = {{ {names} }}, '
                   f'solo = {"true" if solo else "false"}, profile = {"true" if profile else "false"}, trace = {"true" if trace else "false"}, '
                   f'duration = {minutes * 60000} }}\n')


def start(teams=None, seeds=(0, 1, 2), solo=True, lanes=24, minutes=0, target=TARGET, profile=False, trace=False):
    state = call('/api/state').get('run') or {}
    if state.get('state') == 'running':
        return {'error': 'a run is active', 'run': state.get('id')}
    teams = ALL_TEAMS if teams is None else teams
    write_plan(teams, seeds, solo, lanes, minutes, profile, trace)
    suites = [f'brain_lane_{lane:02d}_test' for lane in range(1, lanes + 1)]
    run_id = call('/api/runs', {'kind': 'integration', 'suites': suites, 'retry': 1, 'client': '',
                                'target': target})['id']
    return {'run': run_id, 'teams': len(teams), 'seeds': list(seeds), 'solo': solo, 'target': target}


def progress(run_id):
    state = call('/api/state').get('run') or {}
    if state.get('id') != run_id:
        return {'state': 'finished'}
    suites = state.get('suites', [])
    return {'state': state.get('state'), 'done': sum(1 for suite in suites if suite.get('state') != 'running'),
            'total': len(suites), 'running': [suite['name'] for suite in suites if suite.get('state') == 'running']}


def wait(run_id, limit=3600):
    started = time.time()
    while time.time() - started < limit:
        current = progress(run_id)
        if current['state'] != 'running':
            return current
        time.sleep(2)
    return progress(run_id)


def scan(run_id):
    teams = collections.defaultdict(list)
    solos = []
    errors = set()
    files = {}
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        last = None
        with open(path, 'rb') as log:
            for raw in log:
                if b'failed: ' in raw or b'lua error' in raw:
                    found = FAILED.search(ANSI.sub('', raw.decode('utf-8', 'replace')))
                    if found:
                        errors.add(found.group(0)[:200])
                if b'[sim:' not in raw:
                    continue
                line = ANSI.sub('', raw.decode('utf-8', 'replace'))
                match = TEAM.search(line)
                if match:
                    name, seed, cls, level, died, kills, exp = match.groups()
                    last = (name, int(seed))
                    teams[(name, cls)].append({'seed': int(seed), 'level': int(level), 'died': died == 'true',
                                               'kills': int(kills), 'exp': int(exp)})
                    continue
                match = SOLO.search(line)
                if match:
                    cls, level, died, kills, exp = match.groups()
                    solos.append({'cls': cls, 'level': int(level), 'died': died == 'true', 'kills': int(kills),
                                  'exp': int(exp)})
                    continue
                match = OUTPUT.search(line)
                if match and last is not None and 'team' in match.group(1):
                    files[last] = os.path.join(BOT, match.group(1))
                    last = None
                elif match and 'team' not in match.group(1):
                    files[('solo', len(files))] = os.path.join(BOT, match.group(1))
    return teams, solos, errors, files


def profile_of(run_id, limit=30):
    found = []
    pattern = re.compile(r'\[sim:prof\] (\S+) calls=(\d+) own=([\d.]+)s total=([\d.]+)s')
    wall = None
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        with open(path, 'rb') as log:
            for raw in log:
                if b'[sim:prof]' not in raw:
                    continue
                line = ANSI.sub('', raw.decode('utf-8', 'replace'))
                match = pattern.search(line)
                if match:
                    found.append({'name': match.group(1), 'calls': int(match.group(2)), 'own': float(match.group(3)),
                                  'total': float(match.group(4))})
                elif 'wall=' in line:
                    wall = line.split('wall=')[1].strip()
    return {'wall': wall, 'functions': sorted(found, key=lambda entry: -entry['own'])[:limit]}


def rows(path):
    found = []
    with open(path, encoding='utf-8') as source:
        for line in source:
            try:
                found.append(json.loads(line))
            except ValueError:
                pass
    return found


def episodes(run_id):
    _, _, _, files = scan(run_id)
    return {key: rows(path) for key, path in files.items() if os.path.exists(path)}


def millions(value):
    return round(value / 1000000)


def summary_of(run_id):
    teams, solos, errors, _ = scan(run_id)
    result = {}
    for key in sorted(teams):
        entries = sorted(teams[key], key=lambda entry: entry['seed'])
        result[f'{key[0]} {key[1]}'] = {
            'seeds': [entry['seed'] for entry in entries],
            'kills': [entry['kills'] for entry in entries],
            'exp_m': [millions(entry['exp']) for entry in entries],
            'deaths': sum(1 for entry in entries if entry['died']),
        }
    return result, {
        'profiles': len(solos),
        'kills': sum(entry['kills'] for entry in solos),
        'exp_m': millions(sum(entry['exp'] for entry in solos)),
        'deaths': sum(1 for entry in solos if entry['died']),
        'zero_exp': sum(1 for entry in solos if entry['exp'] == 0),
    } if solos else None, sorted(errors)[:10]


def speed(run_id):
    slowest = []
    actions = 0
    spent = 0.0
    _, _, _, files = scan(run_id)
    for (name, seed), path in files.items():
        if not os.path.exists(path):
            continue
        begun = re.search(r'world_(\d+)_', path)
        count = 0
        with open(path, 'rb') as source:
            for line in source:
                if b'"kind":"act"' in line:
                    count += 1
        wall = os.path.getmtime(path) - int(begun.group(1))
        actions += count
        spent += wall
        slowest.append({'team': name, 'seed': seed, 'seconds': round(wall), 'actions': count,
                        'ms_per_action': round(wall * 1000 / max(count, 1), 1)})
    slowest.sort(key=lambda entry: -entry['seconds'])
    return {'team_seconds': round(spent), 'actions': actions,
            'ms_per_action': round(spent * 1000 / max(actions, 1), 1), 'slowest': slowest[:5]}


def result(run_id, baseline=''):
    teams, solo, errors = summary_of(run_id)
    report = {'progress': progress(run_id), 'teams': teams, 'solo': solo, 'errors': errors, 'speed': speed(run_id)}
    if baseline:
        before, old, _ = summary_of(baseline)
        changes = {}
        for name, entry in teams.items():
            past = before.get(name)
            if past is not None:
                changes[name] = {'exp_m': f"{sum(past['exp_m'])} -> {sum(entry['exp_m'])}",
                                 'kills': f"{sum(past['kills'])} -> {sum(entry['kills'])}",
                                 'deaths': f"{past['deaths']} -> {entry['deaths']}"}
        report['changes'] = changes
        if old is not None and solo is not None:
            report['solo_before'] = old
    return report


def overview(run_id, team='', seed=None):
    found = []
    pattern = re.compile(team) if team else None
    for (name, number), data in sorted(episodes(run_id).items()):
        if (pattern is not None and pattern.search(name) is None) or (seed is not None and seed != number):
            continue
        bots = {}
        for entry in data:
            if entry.get('kind') == 'episode':
                bots[entry['bot']] = {'class': entry.get('class'), 'level': entry.get('level'),
                                      'died': entry.get('died'), 'exp_m': millions(entry.get('exp') or 0),
                                      'kills': (entry.get('stats') or {}).get('kills'),
                                      'casts': {key: entry['casts'][key] for key in WATCHED
                                                if key in (entry.get('casts') or {})},
                                      'used': entry.get('used') or {}, 'seconds': collections.Counter(),
                                      'flees': [], 'supplies': []}
        for entry in data:
            bot = bots.get(entry.get('bot'))
            if bot is None or entry.get('kind') != 'act':
                continue
            begun = entry.get('started', entry['t'])
            bot['seconds'][entry['task']] += (entry['t'] - begun) / 1000
            if entry.get('cause'):
                bot['flees'].append({'t': begun // 1000, 'took': (entry['t'] - begun) // 1000,
                                     'to': entry['map'], 'cause': entry['cause'][:30]})
            elif entry['task'] == 'SUPPLY' and entry.get('action') == 3:
                bot['supplies'].append({'t': begun // 1000, 'took': (entry['t'] - begun) // 1000, 'to': entry['map']})
        stays = {}
        for entry in data:
            if entry.get('kind') == 'stay':
                actual = entry['actual']
                stay = stays.setdefault((entry['bot'], entry['map']), {'map': entry['map'], 'seconds': 0, 'kills': 0,
                                                                        'taken': 0, 'exp_m': 0})
                stay['seconds'] += actual['time'] // 1000
                stay['kills'] += actual['kills']
                stay['taken'] += actual['taken']
                stay['exp_m'] += actual['exp'] / 1000000
        for name_, bot in bots.items():
            bot['seconds'] = {key: round(value) for key, value in bot['seconds'].most_common()}
            mine = [stay for key, stay in stays.items() if key[0] == name_ and stay['seconds'] >= 5]
            bot['grounds'] = sorted(mine, key=lambda stay: -stay['seconds'])[:6]
            for stay in bot['grounds']:
                stay['exp_m'] = round(stay['exp_m'], 1)
        found.append({'team': name, 'seed': number, 'bots': bots})
    return found


def describe(entry):
    item = {'t': entry.get('started', entry['t']) / 1000, 'task': entry['task']}
    for key in ('key', 'item', 'mob', 'map', 'x', 'y', 'hp', 'mp', 'cause', 'reason', 'near', 'values'):
        if entry.get(key) not in (None, ''):
            item[key] = entry[key]
    return item


def timeline(run_id, team, seed, bot='', since=0.0, until=1e9, limit=120, merged=True):
    data = episodes(run_id).get((team, seed))
    if data is None:
        return {'error': 'no such team and seed'}
    acts = [entry for entry in data if entry.get('kind') == 'act' and entry.get('hp') is not None and
            (not bot or entry['bot'] == bot) and since <= entry.get('started', entry['t']) / 1000 <= until]
    if merged:
        spans = []
        for entry in acts:
            key = (entry['bot'], entry['task'], entry['map'])
            if spans and spans[-1]['key'] == key:
                span = spans[-1]
                span['count'] += 1
                span['end'] = entry['t'] / 1000
                span['hp'], span['mp'] = entry['hp'], entry['mp']
                name = entry.get('key') or entry.get('item')
                if name:
                    span['uses'][name] = span['uses'].get(name, 0) + 1
            else:
                name = entry.get('key') or entry.get('item')
                spans.append({'key': key, 'bot': entry['bot'], 'task': entry['task'], 'map': entry['map'],
                              'start': entry.get('started', entry['t']) / 1000, 'end': entry['t'] / 1000,
                              'count': 1, 'hp': entry['hp'], 'mp': entry['mp'], 'uses': {name: 1} if name else {},
                              'cause': (entry.get('cause') or '')[:30]})
        for span in spans:
            del span['key']
        return spans[:limit]
    return [dict(describe(entry), bot=entry['bot']) for entry in acts][:limit]


def deaths(run_id, last=20):
    found = []
    for (name, number), data in sorted(episodes(run_id).items()):
        for entry in data:
            if entry.get('kind') == 'episode' and entry.get('died'):
                acts = [act for act in data if act.get('kind') == 'act' and act['bot'] == entry['bot'] and
                        act.get('hp') is not None]
                found.append({'team': name, 'seed': number, 'bot': entry['bot'], 'class': entry.get('class'),
                              'level': entry.get('level'), 'at': entry.get('time', 0) // 1000,
                              'last': [describe(act) for act in acts[-last:]]})
    return found


def decisions(run_id, bot, hp_low=0, hp_high=10 ** 9, mp_low=0, mp_high=10 ** 9, mob='', limit=20):
    found = []
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        with open(path, 'rb') as log:
            for raw in log:
                if b'[brain:battle]' not in raw or bot.encode() not in raw:
                    continue
                match = BATTLE.search(ANSI.sub('', raw.decode('utf-8', 'replace')))
                if match is None or match.group(1) != bot:
                    continue
                hp, mp = int(match.group(2)), int(match.group(3))
                if hp_low <= hp <= hp_high and mp_low <= mp <= mp_high and (not mob or mob == match.group(4)):
                    found.append({'hp': hp, 'mp': mp, 'mob': match.group(4), 'options': match.group(5),
                                  'picks': match.group(6)})
    if len(found) <= limit:
        return found
    step = len(found) / limit
    return [found[int(index * step)] for index in range(limit)]


def why(run_id, team, seed, kind='why', bot='', limit=10):
    data = episodes(run_id).get((team, seed))
    if data is None:
        return {'error': 'no such team and seed'}
    found = [entry for entry in data if entry.get('kind') == kind and (not bot or entry.get('bot') == bot)]
    return found[:limit]


def calibration(run_id, team=''):
    groups = collections.defaultdict(lambda: [0, 0.0, 0.0, 0.0, 0.0])
    pattern = re.compile(team) if team else None
    for (name, _), data in episodes(run_id).items():
        if pattern is not None and pattern.search(name) is None:
            continue
        for entry in data:
            forecast = entry.get('forecast') if entry.get('kind') == 'fight' else None
            if not isinstance(forecast, dict) or entry.get('result') != 'KILLED':
                continue
            fight = forecast.get('fight') or {}
            actual = entry['actual']
            group = groups[(name, entry['class'], bool(forecast.get('crew')))]
            group[0] += 1
            group[1] += fight.get('taken', 0)
            group[2] += actual.get('taken') if actual.get('taken') is not None else -actual['hp']
            group[3] += fight.get('time', 0)
            group[4] += actual['time']
    return [{'team': key[0], 'class': key[1], 'crew': key[2], 'fights': group[0],
             'taken': f'{group[1] / group[0]:.0f} -> {group[2] / group[0]:.0f}',
             'taken_x': round(group[2] / max(group[1], 1), 2),
             'time': f'{group[3] / group[0] / 1000:.1f}s -> {group[4] / group[0] / 1000:.1f}s',
             'time_x': round(group[4] / max(group[3], 1), 2)} for key, group in sorted(groups.items())]


def brief(run_id, baseline=''):
    outcome = result(run_id, baseline)
    lines = [f"state={outcome['progress'].get('state')} errors={outcome['errors'][:2]}"]
    speed_ = outcome['speed']
    lines.append(f"speed ms_per_action={speed_['ms_per_action']} actions={speed_['actions']} team_seconds={speed_['team_seconds']}")
    lines.append(f"solo {outcome.get('solo')} before {outcome.get('solo_before')}")
    changes = outcome.get('changes') or {}
    for name, team in sorted(outcome['teams'].items()):
        change = changes.get(name, {}).get('exp_m')
        lines.append(f"{name}: kills={team['kills']} exp_m={team['exp_m']} deaths={team['deaths']} {change or ''}")
    for death in deaths(run_id, 4):
        trail = [(step['task'], step.get('key'), step.get('mob'), step['hp'], step['mp']) for step in death['last']]
        lines.append(f"DEATH {death['team']} seed={death['seed']} {death['class']} lv{death['level']} t={death['at']} {trail}")
    return lines


def usage(run_id, team=''):
    pattern = re.compile(team) if team else None
    lines = []
    for (name, number), data in sorted(episodes(run_id).items()):
        if pattern is not None and pattern.search(name) is None:
            continue
        for entry in [row for row in data if row.get('kind') == 'episode']:
            rows_ = [row for row in data if row.get('kind') == 'act' and row['bot'] == entry['bot'] and row.get('hp') is not None]
            if not rows_:
                continue
            top = max(row['hp'] for row in rows_)
            keys = collections.Counter(row.get('key') or row.get('item') for row in rows_ if row.get('key') or row.get('item'))
            spent = [rows_[index - 1]['hp'] / top for index, row in enumerate(rows_)
                     if index > 0 and row.get('key') in STRIKES]
            lines.append(f"{name} {number} {entry['class']} lv{entry.get('level')} exp_m={millions(entry.get('exp') or 0)} "
                         f"strike={len(spent)} below50={sum(1 for ratio in spent if ratio < 0.5)} "
                         f"near90={sum(1 for ratio in spent if ratio >= 0.9)} {dict(keys.most_common(9))}")
    return lines


def places(run_id, team, seed):
    data = episodes(run_id).get((team, seed)) or []
    lines = []
    for bot in sorted({row['bot'] for row in data if row.get('kind') == 'act'}):
        rows_ = [row for row in data if row.get('kind') == 'act' and row['bot'] == bot]
        maps = collections.Counter()
        tasks = collections.Counter()
        for before, after in zip(rows_, rows_[1:]):
            spent = (after['t'] - before['t']) / 1000
            maps[before['map']] += spent
            tasks[(before['task'], before.get('reason'))] += spent
        lines.append(f"{bot} maps={[(key, round(value)) for key, value in maps.most_common(7)]}")
        lines.append(f"{bot} tasks={[(key[0], key[1], round(value)) for key, value in tasks.most_common(10)]}")
    plans = [(row['t'] // 1000, row['bot'], row['detail'].get('name'), round(row['detail'].get('rate') or 0))
             for row in data if row.get('kind') == 'plan']
    lines.append(f"plans={plans[:14]}")
    return lines


def scenarios(suites, target=TARGET, limit=600, retry=0):
    run_id = call('/api/runs', {'kind': 'integration', 'suites': suites, 'retry': retry, 'client': '', 'target': target})['id']
    wait(run_id, limit)
    return scenario_failures(run_id)


def scenario_failures(run_id):
    seen = set()
    lines = [f'run={run_id}']
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        with open(path, 'rb') as log:
            for raw in log:
                if b'FAILED' not in raw and b'SUCCESS' not in raw and b'[brain:' not in raw:
                    continue
                line = ANSI.sub('', raw.decode('utf-8', 'replace')).rstrip()
                match = re.search(r'\[brain:[^\]]+\] (FAILED|SUCCESS).*', line)
                if match is None:
                    continue
                body = match.group(0)
                head = body.split(' history=')[0][:160]
                if head in seen:
                    continue
                seen.add(head)
                if ' FAILED' in head:
                    trail = body.split(' history=')[1].split(' -> ') if ' history=' in body else []
                    counts = collections.Counter(re.sub(r'\(.*', '', step) for step in trail)
                    lines.append(f'{head} {counts.most_common(6)}')
    lines.append(f'passed={sum(1 for key in seen if " SUCCESS" in key)} failed={sum(1 for key in seen if " FAILED" in key)}')
    return lines


def audit_rows(bot_class, rows, died=False):
    keys = collections.Counter()
    top_hp = max((row['hp'] for row in rows), default=1)
    top_mp = max((row['mp'] for row in rows), default=1)
    report = {'class': bot_class, 'died': died}
    swings_after = []
    run = None
    for row in rows:
        key = row.get('key') or row.get('item') or row.get('spell')
        if key:
            keys[key] += 1
        if key in STRIKES:
            if run is not None:
                swings_after.append(run)
            run = 0
        elif key == 'SWING' and run is not None:
            run += 1
    strikes = [row for row in rows if (row.get('key') or row.get('spell')) in STRIKES]
    report['strike'] = len(strikes)
    report['strike_low_hp'] = sum(1 for row in strikes if row['hp'] / max(top_hp, 1) < 0.5)
    report['strike_mp_ratio'] = round(sum(row['mp'] / max(top_mp, 1) for row in strikes) / len(strikes), 2) if strikes else None
    report['swing'] = keys['SWING']
    report['swings_between_strikes'] = round(sum(swings_after) / len(swings_after), 1) if swings_after else None
    if bot_class in ('POET', 'MAGE'):
        drinks = [row for row in rows if row.get('item') in DRINKS]
        report['charge'] = keys['CHARGE'] + keys['공력증강']
        report['drink'] = len(drinks)
        report['drink_above_cost'] = sum(1 for row in drinks if row['mp'] >= 30)
        report['geumgang'] = keys['금강퇴']
    if bot_class == 'POET':
        report['honma'] = keys['혼마술']
    if bot_class == 'MAGE':
        for name in ('마비', '절망', '중독', '저주'):
            report[name] = keys[name]
        report['first'] = sum(count for name, count in keys.items() if "'첨" in name)
    if bot_class == 'ROGUE':
        report['invisible'] = keys['투명']
        report['ambush'] = keys['비영승보']
    if bot_class == 'WARRIOR':
        report['self_heal'] = sum(count for name, count in keys.items() if '기원' in name)
    return report


def apart_ratio(first, second, start, end, step=5000):
    apart = total = 0
    i = j = 0
    now = start
    while now < end:
        while i + 1 < len(first) and first[i + 1][0] <= now:
            i += 1
        while j + 1 < len(second) and second[j + 1][0] <= now:
            j += 1
        total += 1
        apart += first[i][1] != second[j][1]
        now += step
    return round(apart / max(total, 1), 2)


def rules(run_id, team=''):
    pattern = re.compile(team) if team else None
    output = []
    for (name, seed), data in sorted(episodes(run_id).items()):
        if name == 'solo' or (pattern is not None and pattern.search(name) is None):
            continue
        maps = []
        for entry in [row for row in data if row.get('kind') == 'episode']:
            acts = [row for row in data if row.get('kind') == 'act' and row['bot'] == entry['bot']]
            report = audit_rows(entry['class'], [row for row in acts if row.get('hp') is not None], entry.get('died'))
            report['team'] = f'{name}/{seed}'
            report['exp_m'] = millions(entry.get('exp') or 0)
            report['kills'] = (entry.get('stats') or {}).get('kills')
            maps.append([(row['t'], row['map']) for row in acts])
            output.append(report)
        if len(maps) == 2 and maps[0] and maps[1]:
            start = max(maps[0][0][0], maps[1][0][0])
            end = min(maps[0][-1][0], maps[1][-1][0])
            output.append({'team': f'{name}/{seed}', 'apart_ratio': apart_ratio(maps[0], maps[1], start, end)})
    return output


def online_rules(run_id):
    bots = {}
    pattern = re.compile(r'bot=(\S+) class=(\S+) t=(\S+) map=(\S+) task=(\S+) kind=(\S+) key=(\S+) item=(\S+) spell=(\S+) mob=(\S+) hp=(\d+)/\d+ mp=(\d+)/\d+')
    none = lambda value: None if value in ('nil', 'false') else value
    lines = []
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        with open(path, 'rb') as log:
            for raw in log:
                if b'[brain:act]' in raw:
                    lines.append(ANSI.sub('', raw.decode('utf-8', 'replace')).rstrip())
    lines.sort()
    for line in lines:
        match = pattern.search(line)
        if match is None:
            continue
        bot, cls, t, map_, task, _, key, item, spell, _, hp, mp = match.groups()
        bots.setdefault((bot, cls), []).append({'t': float(t), 'map': int(map_), 'task': task, 'key': none(key),
                                                 'item': none(item), 'spell': none(spell), 'hp': int(hp), 'mp': int(mp)})
    output = []
    for (bot, cls), rows_ in bots.items():
        report = audit_rows(cls, rows_)
        report['bot'] = bot[:4]
        report['tasks'] = dict(collections.Counter(row['task'] for row in rows_).most_common(8))
        output.append(report)
    if len(bots) == 2:
        first, second = [[(row['t'], row['map']) for row in rows_] for rows_ in bots.values()]
        output.append({'apart_ratio': apart_ratio(first, second, max(first[0][0], second[0][0]), min(first[-1][0], second[-1][0]))})
    return output


def packets(run_id, limit=12):
    record = call(f'/api/runs/{run_id}')
    elapsed = max(record.get('elapsed') or 1, 1)
    entries = record.get('stats') or []
    groups = collections.Counter()
    for entry in entries:
        groups[entry['name'].split(':')[0]] += entry['count']
    sent = sorted((entry for entry in entries if entry['name'].split(':')[0] in ('game', 'login', 'gateway')),
                  key=lambda entry: -entry['count'])
    total = sum(entry['count'] for entry in sent)
    lines = [f'elapsed={elapsed} groups={groups.most_common()} sent={total} per_second={total / elapsed:.1f}']
    for entry in sent[:limit]:
        lines.append(f"{entry['name']} n={entry['count']} {100 * entry['count'] / max(total, 1):.1f}% p50={entry.get('p50')}")
    for entry in entries:
        if entry['name'] in ('decide:cpu', 'decide:wait', 'brain:decide'):
            lines.append(f"{entry['name']} avg={entry.get('avg')} p50={entry.get('p50')} p95={entry.get('p95')} p99={entry.get('p99')}")
    return lines


def online(run_id, limit=40):
    lines = []
    for path in glob.glob(os.path.join(LOGS, run_id + '-rel-*.log')):
        with open(path, 'rb') as log:
            for raw in log:
                if b'[brain:travel]' in raw or b'brain: ' in raw or b'group:earned' in raw or b'brain:group]' in raw or                         b'[brain:escape]' in raw or b'[brain:plan]' in raw:
                    lines.append(ANSI.sub('', raw.decode('utf-8', 'replace')).rstrip())
    lines.sort()
    bots = {}
    for line in lines:
        match = re.search(r'bot=(\S+) (?:route at|stopped \S+ at|jumped by \S+ to) map=(\d+)', line)
        if match:
            seq = bots.setdefault(match.group(1), [])
            if not seq or seq[-1][1] != match.group(2):
                seq.append((line[22:30], match.group(2), 'jumped' in line))
    output = []
    for bot, seq in bots.items():
        output.append(f"{bot[:4]} hops={len(seq)} jumps={sum(1 for hop in seq if hop[2])} " +
                      ' '.join(f"{hop[0]}:{hop[1]}{'*' if hop[2] else ''}" for hop in seq[:limit]))
    tasks = collections.Counter()
    for line in lines:
        match = re.search(r'brain: bot_id=\d+ \w+/(\w*)/\d+ (\w+)', line)
        if match:
            tasks[f'{match.group(1)}:{match.group(2)}'] += 1
    output.append(f'tasks {tasks.most_common(14)}')
    causes = collections.Counter()
    first = []
    for line in lines:
        match = re.search(r'\[brain:escape\] .*map=(\d+) hp=(\S+) mp=(\S+) cause=(\S+) mob=(\S+) dmg=(\S+) adjacent=(\S+) crew=(\d+)', line)
        if match:
            causes[(match.group(4), match.group(5), match.group(1))] += 1
            if match.group(4) != 'FLEEING' and len(first) < 24:
                first.append(line[22:30] + ' ' + ' '.join(match.groups()))
        elif '[brain:plan]' in line:
            first.append(line[22:30] + ' ' + line[line.find('[brain:plan]') + 13:][:220])
    output.append(f'causes {causes.most_common(12)}')
    output.extend(first)
    output.extend(line[32:200] for line in lines if 'group:earned' in line or 'brain:group]' in line)
    return output


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices=['start', 'wait', 'result', 'rules', 'online-rules', 'packets', 'online', 'scenarios', 'brief', 'usage', 'places', 'overview', 'timeline', 'deaths', 'decisions',
                                            'calibration', 'profile', 'why', 'plan'])
    parser.add_argument('--run', default='')
    parser.add_argument('--baseline', default='')
    parser.add_argument('--teams', nargs='*')
    parser.add_argument('--seeds', nargs='*', type=int, default=[0, 1, 2])
    parser.add_argument('--team', default='')
    parser.add_argument('--seed', type=int)
    parser.add_argument('--bot', default='')
    parser.add_argument('--since', type=float, default=0)
    parser.add_argument('--until', type=float, default=1e9)
    parser.add_argument('--no-solo', action='store_true')
    parser.add_argument('--raw', action='store_true')
    parser.add_argument('--lanes', type=int, default=24)
    parser.add_argument('--minutes', type=int, default=0)
    parser.add_argument('--target', default=TARGET)
    parser.add_argument('--profile', action='store_true')
    parser.add_argument('--trace', action='store_true')
    args = parser.parse_args()
    if args.command == 'start':
        output = start(args.teams, args.seeds, not args.no_solo, args.lanes, args.minutes, args.target, args.profile, args.trace)
    elif args.command == 'wait':
        output = wait(args.run)
    elif args.command == 'result':
        output = result(args.run, args.baseline)
    elif args.command == 'scenarios':
        output = scenarios(args.teams, args.target) if args.teams else scenario_failures(args.run)
    elif args.command == 'rules':
        output = rules(args.run, args.team)
    elif args.command == 'online-rules':
        output = online_rules(args.run)
    elif args.command == 'packets':
        output = packets(args.run)
    elif args.command == 'online':
        output = online(args.run)
    elif args.command == 'brief':
        output = brief(args.run, args.baseline)
    elif args.command == 'usage':
        output = usage(args.run, args.team)
    elif args.command == 'places':
        output = places(args.run, args.team, args.seed)
    elif args.command == 'overview':
        output = overview(args.run, args.team, args.seed)
    elif args.command == 'timeline':
        output = timeline(args.run, args.team, args.seed, args.bot, args.since, args.until, merged=not args.raw)
    elif args.command == 'deaths':
        output = deaths(args.run)
    elif args.command == 'decisions':
        output = decisions(args.run, args.bot)
    elif args.command == 'profile':
        output = profile_of(args.run)
    elif args.command in ('why', 'plan'):
        output = why(args.run, args.team, args.seed, args.command, args.bot)
    else:
        output = calibration(args.run, args.team)
    sys.stdout.reconfigure(encoding='utf-8')
    print(json.dumps(output, ensure_ascii=False, indent=1))


if __name__ == '__main__':
    main()
