import argparse
import json
import time
from pathlib import Path

import health
import server

RECORDS = server.LOGS / 'learning' / 'ramp'


def write(path, entry):
    with open(path, 'a', encoding='utf-8') as file:
        file.write(json.dumps(entry, ensure_ascii=False) + '\n')


def settle(target, timeout, path, previous):
    deadline = time.time() + timeout
    while time.time() < deadline:
        sample = health.snapshot(previous=previous)
        previous = sample
        write(path, {'kind': 'settle', 'target': target, **sample})
        if sample['bots'] >= target * 0.95:
            return sample
        time.sleep(15)
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--stages', default='50,150,300,500')
    parser.add_argument('--hold', type=float, default=5.0)
    parser.add_argument('--offset', type=int, default=20000)
    parser.add_argument('--rate', type=int, default=5)
    parser.add_argument('--settle', type=float, default=15.0)
    parser.add_argument('--rtt', type=float, default=500.0)
    parser.add_argument('--timeouts', type=float, default=0.05)
    parser.add_argument('--cpu', type=float, default=90.0)
    parser.add_argument('--logins', type=int, default=20)
    parser.add_argument('--strikes', type=int, default=3)
    parser.add_argument('--label', default='')
    args = parser.parse_args()

    limits = {'rtt_ms': args.rtt, 'timeout_ratio': args.timeouts, 'cpu_percent': args.cpu,
              'login_failures': args.logins}
    stages = [int(value) for value in args.stages.split(',')]
    RECORDS.mkdir(parents=True, exist_ok=True)
    path = RECORDS / (time.strftime('%Y%m%d-%H%M%S') + (f'-{args.label}' if args.label else '') + '.jsonl')
    write(path, {'kind': 'start', 'stages': stages, 'hold': args.hold, 'offset': args.offset, 'rate': args.rate,
                 'limits': limits, 'time': time.strftime('%Y-%m-%dT%H:%M:%S')})

    started = server.start_load(users=stages[0], minutes=0, offset=args.offset, rate=args.rate)
    write(path, {'kind': 'run', 'response': started})
    if 'error' in started:
        write(path, {'kind': 'end', 'capacity': 0, 'reason': 'start failed'})
        return

    capacity = 0
    reason = 'completed'
    previous = None
    for index, target in enumerate(stages):
        if index > 0:
            write(path, {'kind': 'scale', 'target': target, 'response': server.scale_run(target)})

        settled = settle(target, args.settle * 60, path, previous)
        if settled is not None:
            previous = settled
        if settled is None:
            reason = f'stage {target}: bots did not come online within {args.settle} min'
            break

        strikes = 0
        failed = False
        deadline = time.time() + args.hold * 60
        while time.time() < deadline:
            sample = health.snapshot(previous=previous)
            previous = sample
            reasons = health.verdict(sample, limits)
            if sample['bots'] < target * 0.9:
                reasons.append(f"bots {sample['bots']} < 90% of {target}")
            strikes = strikes + 1 if reasons else 0
            write(path, {'kind': 'hold', 'target': target, 'reasons': reasons, 'strikes': strikes, **sample})
            if strikes >= args.strikes:
                failed = True
                reason = f'stage {target}: ' + '; '.join(reasons)
                break
            time.sleep(15)

        if failed:
            if capacity > 0:
                write(path, {'kind': 'scale', 'target': capacity, 'response': server.scale_run(capacity)})
            break
        capacity = target

    write(path, {'kind': 'end', 'capacity': capacity, 'reason': reason,
                 'time': time.strftime('%Y-%m-%dT%H:%M:%S'), 'stop': server.stop_run()})


if __name__ == '__main__':
    main()
