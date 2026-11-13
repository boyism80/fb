import argparse
import collections
import glob
import json
import os
import re
import subprocess
import sys
import time

import sim

BINARY = os.path.join(sim.BOT, 'Release', 'bot.exe')
LOCAL = os.path.join(sim.LOGS, 'local')


def forward(path):
    return path.replace('\\', '/')


def launch(test, env, log):
    return subprocess.Popen([BINARY, '--mode', 'integration', '--test', test], cwd=sim.BOT, stdout=log,
                            stderr=subprocess.STDOUT, env={**os.environ, **env})


def lanes(teams, seeds=(0,), solo=False, count=0, minutes=5, corpus=False, recent=None, sample=None, stall=None,
          profile=False, audit=0, steps=False, fixed=False, start=None, scene=None):
    run_id = sim.begin(sim.asked(teams, seeds, solo, count, minutes, corpus=corpus, profile=profile, audit=audit or None, steps=steps,
                                 fixed=fixed, recent=recent, sample=sample, stall=stall, start=start, scene=scene))
    started = time.time()
    sim.wait(run_id, 7200)
    return sim.summary(run_id, round(time.time() - started), sim.call(f'/api/runs/{run_id}/result.json'))


def roles(team):
    with open(os.path.join(sim.ROOT, 'scripts', 'sim', 'world', 'roster.lua'), encoding='utf-8') as source:
        text = source.read()
    block = re.search(r'name\s*=\s*"' + re.escape(team) + r'",(.*?)(?:\n\s*name\s*=|\Z)', text, re.S)
    return re.findall(r'class\s*=\s*"(\w+)"', block.group(1)) if block else []


def spots(run_id, team, seed, at):
    stood = {}
    classes = {}
    for entry in sim.episodes(run_id)[(team, seed)]:
        if entry.get('kind') == 'episode':
            classes[entry['bot']] = entry.get('class')
        elif entry.get('kind') == 'act' and entry.get('x') is not None and entry['t'] <= at * 1000:
            stood[entry['bot']] = {'map': entry['map'], 'x': entry['x'], 'y': entry['y']}
    left = dict(stood)
    found = []
    for role in roles(team):
        bot = next((name for name in left if classes.get(name) == role), None)
        if bot is None:
            raise ValueError(f'no {role} of {team} seed {seed} stood anywhere by {at}s in {run_id}')
        found.append(left.pop(bot))
    return found


def scene(run_id='', team='', seed=0, at=0, minutes=2, seeds=(0, 1, 2), start=None, source='', row=0):
    if source:
        with open(source, encoding='utf-8') as lines:
            rows = lines.read().splitlines()
        row = row or len(rows)
        team = team or re.search(r'scene = "(.*?)"', rows[row - 1]).group(1)
        output = lanes([team], seeds, minutes=minutes, scene={'file': forward(os.path.abspath(source)), 'row': row})
        output['scene'] = {'file': source, 'row': row, 'team': team}
    else:
        start = start or spots(run_id, team, seed, at)
        output = lanes([team], seeds, minutes=minutes, start=start)
        output['start'] = start
    return output


def files_of(source):
    if os.path.isdir(source):
        return sorted(glob.glob(os.path.join(source, '*.moments')))
    return sorted(glob.glob(source))


def home_of(source):
    return os.path.join(source, 'redo') if os.path.isdir(source) else os.path.join(LOCAL, 'redo')


def redo(source, shards=4, patch='', row=0, stride=1):
    files = files_of(source)
    home = home_of(source)
    os.makedirs(home, exist_ok=True)
    listing = os.path.join(home, 'list.txt')
    with open(listing, 'w', encoding='utf-8', newline='\n') as target:
        target.write(''.join(forward(name) + '\n' for name in files))
    running = []
    for shard in range(shards):
        out = os.path.join(home, f'out.{shard}.jsonl')
        if os.path.exists(out):
            os.remove(out)
        env = {'MOMENTS_LIST': forward(listing), 'MOMENTS_OUT': forward(out), 'MOMENTS_SHARD': f'{shard}/{shards}',
               'MOMENTS_STRIDE': str(stride)}
        if patch:
            env['MOMENTS_PATCH'] = forward(os.path.abspath(patch))
        if row:
            env['MOMENTS_ROW'] = str(row)
        log = open(os.path.join(home, f'log.{shard}.txt'), 'wb')
        running.append((launch('brain_moments_test', env, log), log, out))
    rows = []
    for process, log, out in running:
        process.wait()
        log.close()
        if os.path.exists(out):
            rows += sim.rows(out)
    return files, rows


def key_of(row):
    return f"{os.path.basename(row['file'])}:{row['row']}"


def leaf(path):
    return (path or 'none').split('>')[-1]


def fidelity(rows):
    known = [row for row in rows if row.get('was') is not None and row.get('now') is not None]
    return {'compared': len(known), 'same': sum(1 for row in known if row['was'] == row['now'])}


def base(source, shards=4):
    started = time.time()
    files, rows = redo(source, shards)
    with open(os.path.join(home_of(source), 'base.jsonl'), 'w', encoding='utf-8', newline='\n') as target:
        for row in rows:
            target.write(json.dumps(row, ensure_ascii=False) + '\n')
    return {'files': len(files), 'moments': len(rows), 'errors': sum(1 for row in rows if row.get('error')),
            'as_played': fidelity(rows), 'seconds': round(time.time() - started)}


def diff(source, shards=4, patch='', stride=1, limit=12):
    started = time.time()
    before = {key_of(row): row for row in sim.rows(os.path.join(home_of(source), 'base.jsonl'))}
    files, rows = redo(source, shards, patch, stride=stride)
    changed = collections.defaultdict(list)
    scenes = collections.Counter()
    missing = 0
    for row in rows:
        old = before.get(key_of(row))
        if old is None:
            missing += 1
        elif old.get('now') != row.get('now'):
            changed[(leaf(old.get('path')), leaf(row.get('path')))].append((old, row))
            scenes[str(row.get('scene'))] += 1
    groups = []
    for (was, now), pairs in sorted(changed.items(), key=lambda entry: -len(entry[1])):
        groups.append({'was': was, 'now': now, 'count': len(pairs),
                       'examples': [{'file': os.path.basename(new['file']), 'row': new['row'], 'scene': new.get('scene'),
                                     'seed': new.get('seed'), 'bot': new.get('bot'), 't': new.get('t'),
                                     'was': old.get('now'), 'now': new.get('now') or new.get('error')}
                                    for old, new in pairs[:3]]})
    total = sum(len(pairs) for pairs in changed.values())
    return {'files': len(files), 'moments': len(rows), 'changed': total, 'not_in_base': missing,
            'errors': sum(1 for row in rows if row.get('error')), 'seconds': round(time.time() - started),
            'by_scene': dict(scenes.most_common()), 'groups': groups[:limit]}


def moment(path, row):
    _, rows = redo(path, 1, row=row)
    return rows[0] if rows else {'error': 'no such row'}


def broken_of(run_id):
    totals = collections.defaultdict(lambda: {'breaks': 0, 'seconds': 0.0, 'longest': 0.0})
    teams = collections.defaultdict(lambda: collections.defaultdict(float))
    worst = collections.defaultdict(list)
    played = 0.0
    for (team, seed), path in sorted(sim.scan(run_id)[3].items()):
        if os.path.exists(path) is False:
            continue
        with open(path, encoding='utf-8') as source:
            for line in source:
                if '"kind":"broke"' in line:
                    row = json.loads(line)
                    lasted = (row['to'] - row['from']) / 1000
                    entry = totals[row['rule']]
                    entry['breaks'] += 1
                    entry['seconds'] += lasted
                    entry['longest'] = max(entry['longest'], lasted)
                    teams[row['rule']][team] += lasted
                elif '"kind":"worst"' in line:
                    row = json.loads(line)
                    worst[row['rule']].append({'team': team, 'seed': seed, 'bot': row.get('bot'), 'from': row['from'] / 1000,
                                               'seconds': (row['to'] - row['from']) / 1000, 'file': row.get('file')})
                elif '"kind":"episode"' in line:
                    played += (json.loads(line).get('time') or 0) / 1000
    return totals, teams, worst, played


def broken(run_id, baseline='', limit=3):
    totals, teams, worst, played = broken_of(run_id)
    before = broken_of(baseline)[0] if baseline else {}
    found = []
    for rule in sorted(set(totals) | set(before)):
        entry = totals.get(rule, {'breaks': 0, 'seconds': 0.0, 'longest': 0.0})
        row = {'rule': rule, 'breaks': entry['breaks'], 'seconds': round(entry['seconds'], 1), 'longest': round(entry['longest'], 1),
               'teams': {team: round(seconds, 1) for team, seconds in sorted(teams[rule].items(), key=lambda item: -item[1])[:limit]},
               'worst': sorted(worst[rule], key=lambda item: -item['seconds'])[:limit]}
        if baseline:
            row['was'] = {'breaks': before.get(rule, {}).get('breaks', 0), 'seconds': round(before.get(rule, {}).get('seconds', 0.0), 1)}
        found.append(row)
    return {'run': run_id, 'bot_seconds': round(played), 'rules': found}


def played(run_id):
    listed = os.path.join(sim.RUNS, run_id, 'result.json')
    if os.path.exists(listed):
        with open(listed, encoding='utf-8') as source:
            found = json.load(source)
        files = {f"{row['team']}#{row['seed']}": row['file'] for row in found.get('teams') or []}
        files.update({f"solo {row['profile']}#{row['seed']}": row['file'] for row in found.get('solos') or []})
    else:
        files = {f'{key[0]}#{key[1]}': path for key, path in sim.scan(run_id)[3].items()}
    return {key: path for key, path in files.items() if os.path.exists(path)}


def stable(row):
    kept = {name: value for name, value in row.items() if name != 'file'}
    if isinstance(kept.get('forecast'), dict):
        kept['forecast'] = {name: value for name, value in kept['forecast'].items() if name != 'snapshot'}
    return kept


def ordered(rows):
    kept = [stable(row) for row in rows]
    played = [row for row in kept if row.get('kind') not in ('broke', 'worst')]
    watched = sorted((row for row in kept if row.get('kind') in ('broke', 'worst')), key=lambda row: json.dumps(row, sort_keys=True))
    return played + watched


def same(run_id, baseline, limit=6):
    now = played(run_id)
    was = played(baseline)
    apart = []
    rows = 0
    for key in sorted(set(now) & set(was)):
        left = ordered(sim.rows(was[key]))
        right = ordered(sim.rows(now[key]))
        rows += len(right)
        first = next((index for index, pair in enumerate(zip(left, right)) if pair[0] != pair[1]), None)
        if first is None and len(left) != len(right):
            first = min(len(left), len(right))
        if first is not None:
            before = left[first] if first < len(left) else {}
            after = right[first] if first < len(right) else {}
            fields = sorted(name for name in set(before) | set(after) if before.get(name) != after.get(name))
            apart.append({'episode': key, 'row': first, 'of': [len(left), len(right)], 'kind': after.get('kind') or before.get('kind'),
                          'bot': after.get('bot') or before.get('bot'), 't': after.get('t', before.get('t')), 'fields': fields[:8]})
    return {'run': run_id, 'baseline': baseline, 'episodes': len(set(now) & set(was)), 'only_one_side': len(set(now) ^ set(was)),
            'rows': rows, 'different': len(apart), 'first': apart[:limit]}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices=['lanes', 'base', 'diff', 'moment', 'broken', 'same', 'scene'])
    parser.add_argument('--team', default='')
    parser.add_argument('--seed', type=int, default=0)
    parser.add_argument('--at', type=float, default=0)
    parser.add_argument('--teams', nargs='*', default=[])
    parser.add_argument('--seeds', nargs='*', type=int, default=[0])
    parser.add_argument('--solo', action='store_true')
    parser.add_argument('--count', type=int, default=0)
    parser.add_argument('--minutes', type=float, default=5)
    parser.add_argument('--corpus', action='store_true')
    parser.add_argument('--profile', action='store_true')
    parser.add_argument('--audit', type=int, default=0)
    parser.add_argument('--steps', action='store_true')
    parser.add_argument('--fixed', action='store_true')
    parser.add_argument('--source', default='')
    parser.add_argument('--shards', type=int, default=4)
    parser.add_argument('--patch', default='')
    parser.add_argument('--row', type=int, default=0)
    parser.add_argument('--stride', type=int, default=1)
    parser.add_argument('--run', default='')
    parser.add_argument('--baseline', default='')
    args = parser.parse_args()
    if args.command == 'lanes':
        output = lanes(args.teams, args.seeds, args.solo, args.count, args.minutes, args.corpus, profile=args.profile,
                       audit=args.audit, steps=args.steps, fixed=args.fixed)
    elif args.command == 'base':
        output = base(args.source, args.shards)
    elif args.command == 'diff':
        output = diff(args.source, args.shards, args.patch, args.stride)
    elif args.command == 'same':
        output = same(args.run, args.baseline)
    elif args.command == 'broken':
        output = broken(args.run, args.baseline)
    elif args.command == 'scene':
        output = scene(args.run, args.team, args.seed, args.at, args.minutes, args.seeds, source=args.source, row=args.row)
    else:
        output = moment(args.source, args.row)
    sys.stdout.reconfigure(encoding='utf-8')
    print(json.dumps(output, ensure_ascii=False, indent=1))


if __name__ == '__main__':
    main()
