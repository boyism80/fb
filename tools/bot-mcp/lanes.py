import argparse
import json
import sys

import sim


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--teams', nargs='*', default=[])
    parser.add_argument('--seeds', nargs='*', type=int, default=[0, 1, 2])
    parser.add_argument('--solo', action='store_true')
    parser.add_argument('--lanes', type=int, default=24)
    parser.add_argument('--minutes', type=int, default=0)
    parser.add_argument('--target', default=sim.TARGET)
    parser.add_argument('--limit', type=int, default=3600)
    parser.add_argument('--baseline', default='')
    args = parser.parse_args()
    started = sim.start(args.teams, args.seeds, args.solo, args.lanes, args.minutes, args.target)
    if 'error' in started:
        print(started)
        return
    sim.wait(started['run'], args.limit)
    sys.stdout.reconfigure(encoding='utf-8')
    print(f"run={started['run']}")
    print(chr(10).join(sim.brief(started['run'], args.baseline)))


if __name__ == '__main__':
    main()
