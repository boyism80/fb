import json
import subprocess
import time

import server

PROBE = 'rtt:info'


def processes(seconds=2.0):
    script = (
        "$w = [Diagnostics.Stopwatch]::StartNew(); "
        "$a = Get-Process | Where-Object { $_.Path -like 'D:\\git\\fb\\server\\*' } | "
        "Select-Object Id, ProcessName, @{n='cpu';e={$_.TotalProcessorTime.TotalMilliseconds}}, WorkingSet64; "
        "$t0 = $w.ElapsedMilliseconds; "
        f"$m = (Get-Counter '\\Processor(_Total)\\% Processor Time' -SampleInterval {max(1, int(seconds))} -MaxSamples 1)"
        ".CounterSamples[0].CookedValue; "
        "$b = Get-Process | Where-Object { $_.Path -like 'D:\\git\\fb\\server\\*' } | "
        "Select-Object Id, ProcessName, @{n='cpu';e={$_.TotalProcessorTime.TotalMilliseconds}}, WorkingSet64; "
        "$t1 = $w.ElapsedMilliseconds; "
        "@{a=$a; b=$b; t0=$t0; t1=$t1; machine=$m; cores=[Environment]::ProcessorCount} | ConvertTo-Json -Depth 3 -Compress"
    )
    output = subprocess.run(['powershell', '-NoProfile', '-Command', script],
                            capture_output=True, text=True, timeout=60).stdout
    data = json.loads(output or '{}')
    before = {entry['Id']: entry for entry in data.get('a') or []}
    cores = data.get('cores') or 1
    elapsed = max(float(data.get('t1', 0)) - float(data.get('t0', 0)), seconds * 1000)
    result = {}
    for entry in data.get('b') or []:
        name = entry['ProcessName']
        old = before.get(entry['Id'])
        cpu = (entry['cpu'] - old['cpu']) / elapsed / cores * 100 if old else 0.0
        bucket = result.setdefault(name, {'count': 0, 'cpu': 0.0, 'memory_mb': 0.0})
        bucket['count'] += 1
        bucket['cpu'] = round(bucket['cpu'] + cpu, 1)
        bucket['memory_mb'] = round(bucket['memory_mb'] + entry['WorkingSet64'] / 1048576, 1)
    return result, round(float(data.get('machine') or 0.0), 1)


def window(timeline, seconds):
    if not timeline:
        return []
    last = timeline[-1].get('t', 0)
    return [point for point in timeline if point.get('t', 0) >= last - seconds]


def probe(stats, previous, name=PROBE):
    entry = stats.get(name, {})
    count = entry.get('count') or 0
    failures = entry.get('failures') or 0
    average = entry.get('avg') or 0.0
    old = previous or {}
    delta = count - (old.get('count') or 0)
    if delta <= 0:
        return {'count': count, 'failures': failures, 'avg': average, 'window_count': 0, 'window_avg': None,
                'window_timeouts': None, 'p95': entry.get('p95'), 'p99': entry.get('p99')}
    total = average * count - (old.get('avg') or 0.0) * (old.get('count') or 0)
    return {'count': count, 'failures': failures, 'avg': average, 'window_count': delta,
            'window_avg': round(total / delta, 1),
            'window_timeouts': round((failures - (old.get('failures') or 0)) / delta, 3),
            'p95': entry.get('p95'), 'p99': entry.get('p99')}


def snapshot(window_seconds=30, previous=None):
    state = server.call('/api/state')
    run = state.get('run') or {}
    timeline = run.get('timeline') or []
    recent = window(timeline, window_seconds)
    stats = {entry['name']: entry for entry in run.get('stats') or []}
    logins = [entry for name, entry in stats.items() if name.startswith('login:')]
    usage, machine = processes()
    return {
        'time': time.strftime('%Y-%m-%dT%H:%M:%S'),
        'run': run.get('id'),
        'state': run.get('state'),
        'users': run.get('users'),
        'elapsed': run.get('elapsed'),
        'bots': timeline[-1].get('bots') if timeline else 0,
        'agents': timeline[-1].get('agents') if timeline else 0,
        'rps': round(sum(point.get('rps', 0) for point in recent) / max(len(recent), 1), 1),
        'rtt': probe(stats, (previous or {}).get('rtt')),
        'lag': probe(stats, (previous or {}).get('lag'), 'lag:loop'),
        'login_failures': sum(entry.get('failures', 0) for entry in logins),
        'deaths': timeline[-1].get('deaths') if timeline else 0,
        'kills': timeline[-1].get('kills') if timeline else 0,
        'processes': usage,
        'cpu_fb': round(sum(value['cpu'] for value in usage.values()), 1),
        'cpu_machine': machine,
    }


def verdict(sample, limits):
    reasons = []
    rtt = sample['rtt']
    if rtt['window_avg'] is not None and rtt['window_avg'] > limits['rtt_ms']:
        reasons.append(f"rtt avg {rtt['window_avg']}ms > {limits['rtt_ms']}")
    if rtt['window_timeouts'] is not None and rtt['window_timeouts'] > limits['timeout_ratio']:
        reasons.append(f"rtt timeouts {rtt['window_timeouts']} > {limits['timeout_ratio']}")
    if sample['cpu_machine'] > limits['cpu_percent']:
        reasons.append(f"machine cpu {sample['cpu_machine']}% > {limits['cpu_percent']}")
    if sample['login_failures'] > limits['login_failures']:
        reasons.append(f"login failures {sample['login_failures']}")
    return reasons


if __name__ == '__main__':
    print(json.dumps(snapshot(), ensure_ascii=False, indent=1))
