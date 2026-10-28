import collections
import datetime
import json
import os
import shutil
import socket
import subprocess
import urllib.error
import urllib.request
from pathlib import Path

from mcp.server.fastmcp import FastMCP

ROOT = Path(__file__).resolve().parents[2]
BOT = ROOT / 'server' / 'bot'
LOGS = BOT / 'logs'
DATA = BOT / 'config' / 'load.json'
MASTER = os.environ.get('BOT_MASTER', 'http://127.0.0.1:8089')
PORTS = {'gateway': 3001, 'game': 3004, 'game-cross-0': 3100, 'game-cross-1': 3101, 'game-cross-2': 3102, 'master': 8089}

server = FastMCP('fb-bot')


def call(path, body=None):
    data = None if body is None else json.dumps(body).encode('utf-8')
    request = urllib.request.Request(MASTER + path, data=data, method='GET' if body is None else 'POST')
    request.add_header('Content-Type', 'application/json')
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            return json.loads(response.read().decode('utf-8') or '{}')
    except urllib.error.HTTPError as error:
        return {'error': error.read().decode('utf-8', 'replace'), 'status': error.code}
    except urllib.error.URLError as error:
        return {'error': f'master unreachable: {error.reason}'}


def stamp(text):
    try:
        return datetime.datetime.fromisoformat(text[:26])
    except (TypeError, ValueError):
        return None


def events(since_minutes=None, folder=None):
    cutoff = None
    if since_minutes:
        cutoff = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None) - datetime.timedelta(minutes=since_minutes)
    for path in sorted((folder or LOGS).glob('load-trace-*.jsonl')):
        with open(path, encoding='utf-8', errors='replace') as file:
            for line in file:
                try:
                    entry = json.loads(line)
                except ValueError:
                    continue
                if cutoff is not None:
                    when = stamp(entry.get('t', ''))
                    if when is None or when < cutoff:
                        continue
                yield entry


@server.tool()
def cluster_state() -> dict:
    """Master snapshot: connected agents, their load, and the current run."""
    return call('/api/state')


@server.tool()
def start_load(users: int, minutes: int = 10, offset: int = 0, rate: int = 1, client: str = '') -> dict:
    """Start a load run on the cluster. minutes is the run length (0 runs until stop_run), offset is the bot account offset (fresh accounts need a new range)."""
    return call('/api/runs', {'kind': 'load', 'users': users, 'duration': minutes, 'offset': offset, 'rate': rate, 'client': client})


@server.tool()
def start_integration(suites: list[str], retry: int = 1, client: str = '') -> dict:
    """Start integration suites (names as listed by list_suites) on the cluster."""
    return call('/api/runs', {'kind': 'integration', 'suites': suites, 'retry': retry, 'client': client})


@server.tool()
def list_suites() -> dict:
    """Integration suites the master knows."""
    return call('/api/suites')


@server.tool()
def stop_run() -> dict:
    """Stop the current run."""
    return call('/api/runs/current/stop', {})


@server.tool()
def scale_run(users: int) -> dict:
    """Change the number of bots in the current load run."""
    return call('/api/runs/current/scale', {'users': users})


@server.tool()
def run_history() -> dict:
    """Finished runs kept by the master."""
    return call('/api/runs')


@server.tool()
def run_summary(run_id: str, points: int = 20) -> dict:
    """Summary of one run; the timeline is cut to the last `points` samples."""
    result = call(f'/api/runs/{run_id}')
    if isinstance(result.get('timeline'), list):
        result['timeline'] = result['timeline'][-points:]
    return result


@server.tool()
def servers_status() -> dict:
    """Whether the FB servers and the master listen, and which FB processes run."""
    ports = {}
    for name, port in PORTS.items():
        with socket.socket() as probe:
            probe.settimeout(0.5)
            ports[name] = probe.connect_ex(('127.0.0.1', port)) == 0
    command = ['powershell', '-NoProfile', '-Command',
               "Get-Process | Where-Object { $_.Path -like 'D:\\git\\fb\\server\\*' } | "
               "ForEach-Object { '{0} {1} {2:o}' -f $_.ProcessName, $_.Id, $_.StartTime }"]
    output = subprocess.run(command, capture_output=True, text=True, timeout=20).stdout
    return {'ports': ports, 'processes': [line for line in output.splitlines() if line.strip()]}


@server.tool()
def bots(since_minutes: float = 60) -> dict:
    """Latest state of every bot: level (from the state event and from level-up messages), goal, map, hp, money, deaths."""
    result = {}
    ups = collections.Counter()
    deaths = collections.Counter()
    for entry in events(since_minutes):
        name = entry.get('bot')
        if not name:
            continue
        if entry.get('kind') == 'LEVEL_UP':
            ups[name] += 1
        if entry.get('kind') == 'EXP_LOST':
            deaths[name] += 1
        if entry.get('event') == 'state':
            result[name] = {key: entry.get(key) for key in
                            ('t', 'level', 'class', 'goal', 'map', 'x', 'y', 'hp', 'max_hp', 'mp', 'money', 'items',
                             'ghost', 'leader', 'partner', 'quest', 'ground')}
    for name in set(ups) | set(deaths) | set(result):
        result.setdefault(name, {})
        result[name]['level_ups'] = ups[name]
        result[name]['deaths'] = deaths[name]
    return result


@server.tool()
def bot_events(name: str, limit: int = 40, kinds: list[str] | None = None, skip_chat: bool = True) -> list:
    """Most recent trace events of one bot. kinds filters the event field (state, goal, result, action, interrupt, stuck, message...)."""
    found = collections.deque(maxlen=limit)
    for entry in events():
        if entry.get('bot') != name:
            continue
        kind = entry.get('event')
        if skip_chat and kind in ('chat',):
            continue
        if kinds and kind not in kinds:
            continue
        found.append(entry)
    return list(found)


@server.tool()
def trace_summary(since_minutes: float = 30) -> dict:
    """Aggregates over the trace: goal results, action outcomes, interrupts, travel failures, quests, level-ups, deaths by map."""
    goals = collections.defaultdict(collections.Counter)
    actions = collections.defaultdict(collections.Counter)
    interrupts = collections.Counter()
    travel = collections.Counter()
    quests = collections.Counter()
    ups = collections.Counter()
    death_maps = collections.Counter()
    stuck = collections.Counter()
    configs = []
    last_map = {}
    for entry in events(since_minutes):
        kind = entry.get('event')
        name = entry.get('bot', '')
        if 'map' in entry:
            last_map[name] = entry['map']
        if kind == 'result':
            goals[entry.get('goal')][entry.get('result')] += 1
        elif kind == 'action':
            action = entry.get('action')
            actions[action][entry.get('reason') or 'OK'] += 1
            if action == 'travel':
                travel[f"{entry.get('reason')} {entry.get('from')}->{entry.get('to')}"] += 1
            if action == 'quest':
                quests[f"{entry.get('id')} {entry.get('reason') or 'OK'}"] += 1
        elif kind == 'interrupt':
            interrupts[f"{entry.get('from')}->{entry.get('to')}"] += 1
        elif kind == 'stuck':
            stuck[f"{entry.get('goal')} map {entry.get('map')}"] += 1
        elif kind == 'config':
            configs.append({key: entry.get(key) for key in ('t', 'hash', 'users', 'offset')})
        if entry.get('kind') == 'LEVEL_UP':
            ups[name] += 1
        if entry.get('kind') == 'EXP_LOST':
            death_maps[last_map.get(name)] += 1
    return {
        'configs': configs[-5:],
        'goals': {goal: dict(counts) for goal, counts in goals.items()},
        'actions': {action: dict(counts) for action, counts in actions.items()},
        'interrupts': dict(interrupts.most_common(15)),
        'travel_failures': dict(travel.most_common(15)),
        'quests': dict(quests),
        'level_ups': dict(ups),
        'deaths_by_map': dict(death_maps),
        'stuck': dict(stuck),
    }


ONE_SHOT_GOALS = {'JOB', 'LEARN', 'EQUIP', 'QUEST', 'RECLAIM', 'REVIVE', 'SUPPLY', 'SELL', 'TRADE'}


def rates(folder=None):
    counts = collections.Counter()
    first = last = None
    for entry in events(folder=folder):
        when = stamp(entry.get('t', ''))
        if when is not None:
            first = when if first is None or when < first else first
            last = when if last is None or when > last else last
        kind = entry.get('event')
        if kind in ('state', 'chat', 'config', 'quest_pick', 'goal'):
            continue
        if kind == 'action':
            key = f"action {entry.get('action')} {entry.get('reason') or entry.get('outcome') or 'OK'}"
        elif kind == 'message':
            key = f"message {entry.get('kind')}"
        elif kind == 'result':
            key = f"result {entry.get('goal')} {entry.get('result')}"
        else:
            key = kind
        counts[key] += 1
    minutes = max((last - first).total_seconds() / 60, 1.0) if first and last else 1.0
    return {key: count / minutes for key, count in counts.items()}, minutes


@server.tool()
def trace_diff(baseline: str = '', ratio: float = 2.0, minimum: int = 10) -> dict:
    """Per-minute event rates of the current trace against an archived run (logs/archive/<baseline>, default the newest).
    Lists every key that rose by at least ratio with at least minimum occurrences, and keys that are new."""
    archive = LOGS / 'archive'
    folders = sorted(path for path in archive.iterdir() if path.is_dir()) if archive.exists() else []
    folder = archive / baseline if baseline else (folders[-1] if folders else None)
    if folder is None or folder.exists() is False:
        return {'error': f'no baseline {baseline}'}
    now, now_minutes = rates()
    before, before_minutes = rates(folder)
    rising = []
    for key, rate in now.items():
        count = rate * now_minutes
        if count < minimum:
            continue
        old = before.get(key, 0.0)
        if old == 0.0 or rate / old >= ratio:
            rising.append({'key': key, 'per_min': round(rate, 2), 'baseline_per_min': round(old, 2), 'count': round(count)})
    rising.sort(key=lambda item: item['per_min'] / max(item['baseline_per_min'], 0.01), reverse=True)
    return {'baseline': folder.name, 'minutes': round(now_minutes, 1), 'baseline_minutes': round(before_minutes, 1), 'rising': rising}


def capped(state):
    return state is not None and state.get('class') in (None, '', 'NONE', '평민') and (state.get('level') or 0) >= 5


@server.tool()
def anomalies(since_minutes: float = 30, stall_minutes: float = 10, repeat: int = 5, grace_minutes: float = 5,
              hollow_limit: int = 15) -> list:
    """Rule-based problems: bots without level-ups, repeated identical failures, goal flapping, death clusters, stuck goals, silent trace,
    warp loops (hazard events), one-shot goals that keep succeeding (hollow_success), items left after loot.
    stall skips class-less bots at the level-5 cap and starts counting grace_minutes after a bot's session begins."""
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    last_up = {}
    last_seen = {}
    failures = collections.Counter()
    flaps = collections.Counter()
    deaths = collections.Counter()
    stuck = collections.Counter()
    hazards = collections.Counter()
    hollow = collections.Counter()
    leftovers = collections.Counter()
    last_map = {}
    latest_state = {}
    for entry in events(since_minutes):
        name = entry.get('bot')
        when = stamp(entry.get('t', ''))
        if not name or when is None:
            continue
        if name not in last_seen or (when - last_seen[name]).total_seconds() > 120:
            last_up[name] = when + datetime.timedelta(minutes=grace_minutes)
        last_seen[name] = when
        if entry.get('event') == 'state':
            latest_state[name] = entry
        if 'map' in entry:
            last_map[name] = entry['map']
        if entry.get('kind') == 'LEVEL_UP':
            last_up[name] = max(last_up[name], when)
        if entry.get('kind') == 'EXP_LOST':
            deaths[last_map.get(name)] += 1
        if entry.get('event') == 'action' and entry.get('reason'):
            failures[(name, entry.get('action'), entry.get('reason'))] += 1
        if entry.get('event') == 'interrupt':
            flaps[(name, entry.get('from'), entry.get('to'))] += 1
        if entry.get('event') == 'stuck':
            stuck[(name, entry.get('goal'), entry.get('map'))] += 1
        if entry.get('event') == 'hazard':
            hazards[(name, entry.get('from'))] += 1
        if entry.get('event') == 'result' and entry.get('result') == 'SUCCEEDED' and entry.get('goal') in ONE_SHOT_GOALS:
            hollow[(name, entry.get('goal'))] += 1
        if entry.get('event') == 'action' and entry.get('action') == 'loot' and (entry.get('left') or 0) > 0:
            leftovers[name] += 1

    found = []
    for name, seen in last_seen.items():
        if (now - seen).total_seconds() > 120:
            found.append({'type': 'silent', 'bot': name, 'detail': f'no trace for {int((now - seen).total_seconds())}s'})
        elif capped(latest_state.get(name)):
            continue
        elif (seen - last_up[name]).total_seconds() > stall_minutes * 60:
            found.append({'type': 'stall', 'bot': name,
                          'detail': f'no level-up for {int((seen - last_up[name]).total_seconds() / 60)} min'})
    for (name, action, reason), count in failures.items():
        if count >= repeat:
            found.append({'type': 'repeat_failure', 'bot': name, 'detail': f'{action} {reason} x{count}'})
    for (name, source, target), count in flaps.items():
        if count >= repeat:
            found.append({'type': 'flapping', 'bot': name, 'detail': f'{source} interrupted by {target} x{count}'})
    for (name, map_id), count in hazards.items():
        if count >= repeat:
            found.append({'type': 'warp_loop', 'bot': name, 'detail': f'left map {map_id} by stepping on a warp x{count}'})
    for (name, goal), count in hollow.items():
        if count >= hollow_limit:
            found.append({'type': 'hollow_success', 'bot': name, 'detail': f'{goal} succeeded x{count}'})
    for name, count in leftovers.items():
        if count >= repeat:
            found.append({'type': 'loot_leftover', 'bot': name, 'detail': f'items left on the tile after loot x{count}'})
    for map_id, count in deaths.items():
        if count >= 3:
            found.append({'type': 'death_cluster', 'map': map_id, 'detail': f'{count} deaths'})
    for (name, goal, map_id), count in stuck.items():
        found.append({'type': 'stuck', 'bot': name, 'detail': f'{goal} on map {map_id} x{count}'})
    if not last_seen:
        found.append({'type': 'silent', 'detail': 'no trace events in the window (servers down or no run)'})
    return found


@server.tool()
def health(window_seconds: int = 30) -> dict:
    """Live health: online bots, server round trip (rtt:info, self_info every 10s per bot), login failures, CPU and memory."""
    import health as probe
    return probe.snapshot(window_seconds)


@server.tool()
def ramp_start(stages: str = '50,150,300,500', hold_minutes: float = 5, offset: int = 20000, rate: int = 5,
               rtt_ms: float = 500, timeout_ratio: float = 0.05, cpu_percent: float = 90, label: str = '') -> dict:
    """Start a background ramp: raise bots stage by stage (1x speed expected in load.json), hold each stage, and on repeated
    health breaches scale back to the last good stage and stop. Progress goes to logs/learning/ramp/*.jsonl."""
    command = ['python', str(Path(__file__).with_name('ramp.py')), '--stages', stages, '--hold', str(hold_minutes),
               '--offset', str(offset), '--rate', str(rate), '--rtt', str(rtt_ms), '--timeouts', str(timeout_ratio),
               '--cpu', str(cpu_percent), '--label', label]
    flags = getattr(subprocess, 'CREATE_NEW_PROCESS_GROUP', 0) | getattr(subprocess, 'DETACHED_PROCESS', 0)
    process = subprocess.Popen(command, cwd=str(Path(__file__).parent), creationflags=flags,
                               stdout=subprocess.DEVNULL, stderr=open(LOGS / 'learning' / 'ramp-error.log', 'a'))
    return {'pid': process.pid, 'command': command}


@server.tool()
def ramp_status(lines: int = 6) -> dict:
    """Latest ramp record: settings, the last samples (stage, bots, p95/p99, cpu, breach reasons), and the end result."""
    folder = LOGS / 'learning' / 'ramp'
    files = sorted(folder.glob('*.jsonl')) if folder.exists() else []
    if not files:
        return {'error': 'no ramp record'}
    entries = [json.loads(line) for line in files[-1].read_text(encoding='utf-8').splitlines() if line.strip()]
    brief = []
    for entry in entries[-lines:]:
        brief.append({key: entry.get(key) for key in
                      ('kind', 'time', 'target', 'bots', 'rtt', 'cpu_machine', 'cpu_fb', 'login_failures',
                       'reasons', 'strikes', 'capacity', 'reason')})
    return {'file': files[-1].name, 'start': entries[0], 'recent': brief,
            'end': next((entry for entry in entries if entry.get('kind') == 'end'), None)}


@server.tool()
def archive_traces(label: str = '') -> dict:
    """Move the current load traces into logs/archive/<time>-<label>/ so the next run starts clean."""
    target = LOGS / 'archive' / (datetime.datetime.now().strftime('%Y%m%d-%H%M%S') + (f'-{label}' if label else ''))
    target.mkdir(parents=True, exist_ok=True)
    moved = []
    for path in LOGS.glob('load-trace-*.jsonl'):
        try:
            shutil.move(str(path), target / path.name)
            moved.append(path.name)
        except OSError as error:
            moved.append(f'{path.name}: {error}')
    return {'archive': str(target), 'moved': moved}


def walk(document, path):
    node = document
    for key in path.split('.'):
        if isinstance(node, list):
            node = node[int(key)]
        else:
            node = node[key]
    return node


@server.tool()
def config_get(path: str = '') -> object:
    """Read config/load.json, or one value by dotted path (personas.0.weights.quest, spells.weights.HEAL)."""
    document = json.loads(DATA.read_text(encoding='utf-8'))
    return walk(document, path) if path else document


@server.tool()
def config_set(path: str, value: object) -> dict:
    """Set one value in config/load.json by dotted path. Runs started after this read the new value."""
    document = json.loads(DATA.read_text(encoding='utf-8'))
    parent_path, _, key = path.rpartition('.')
    parent = walk(document, parent_path) if parent_path else document
    if isinstance(parent, list):
        before = parent[int(key)]
        parent[int(key)] = value
    else:
        before = parent.get(key)
        parent[key] = value
    DATA.write_text(json.dumps(document, ensure_ascii=False, indent='\t') + '\n', encoding='utf-8')
    return {'path': path, 'before': before, 'after': value}


if __name__ == '__main__':
    server.run()
