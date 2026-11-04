import collections
import datetime
import json
import statistics
from pathlib import Path


def stamp(text):
    if isinstance(text, (int, float)):
        return datetime.datetime(2000, 1, 1) + datetime.timedelta(milliseconds=text)
    try:
        return datetime.datetime.fromisoformat(text[:23])
    except (TypeError, ValueError):
        return None


def records(folder, since_minutes=None, kinds=None):
    cutoff = None
    if since_minutes:
        cutoff = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None) - datetime.timedelta(minutes=since_minutes)
    for path in sorted(Path(folder).glob('*.jsonl')):
        with open(path, encoding='utf-8', errors='replace') as file:
            for line in file:
                try:
                    entry = json.loads(line)
                except ValueError:
                    continue
                if kinds is not None and entry.get('kind') not in kinds:
                    continue
                when = stamp(entry.get('t', ''))
                if cutoff is not None and isinstance(entry.get('t'), str) and (when is None or when < cutoff):
                    continue
                entry['_when'] = when
                yield entry


def median(values):
    values = [value for value in values if value is not None and value >= 0]
    return statistics.median(values) if values else None


def ratio(actual, predicted):
    if actual is None or predicted is None or predicted <= 0:
        return None
    return actual / predicted


def band(level):
    return f'{((level or 1) - 1) // 10 * 10 + 1}-{((level or 1) - 1) // 10 * 10 + 10}'


def casts(uses):
    return sum(count for key, count in uses.items() if key != 'SWING' and not key.startswith('ACT:'))


def summarize_fights(fights, minimum, tolerance):
    groups = collections.defaultdict(list)
    for fight in fights:
        forecast = fight.get('forecast') or {}
        key = (fight.get('class'), band(fight.get('level')), fight.get('mob'), forecast.get('key'))
        groups[key].append(fight)
    rows = []
    for (cls, levels, mob, opener), items in groups.items():
        killed = [item for item in items if item.get('result') == 'KILLED']
        died = sum(1 for item in items if item.get('result') == 'DIED')
        actual_time = median([item['actual']['time'] for item in killed])
        fight_time = median([(item.get('forecast') or {}).get('fight', {}).get('time') for item in killed])
        model_time = median([(item.get('forecast') or {}).get('model', {}).get('time') for item in killed])
        actual_taken = median([max(0, -(item['actual']['hp'])) for item in killed])
        fight_taken = median([max(0, ((item.get('forecast') or {}).get('fight', {}).get('taken') or 0) -
                                  ((item.get('forecast') or {}).get('fight', {}).get('healed') or 0)) for item in killed])
        actual_casts = median([casts(item['actual'].get('uses') or {}) for item in killed])
        model_casts = median([(item.get('forecast') or {}).get('model', {}).get('casts') for item in killed])
        actual_swings = median([(item['actual'].get('uses') or {}).get('SWING', 0) for item in killed])
        model_swings = median([(item.get('forecast') or {}).get('model', {}).get('swings') for item in killed])
        predicted_death = statistics.mean([(item.get('forecast') or {}).get('fight', {}).get('p_die') or 0 for item in items])
        opener_unused = 0
        if opener not in (None, 'SWING'):
            opener_unused = sum(1 for item in killed if (item['actual'].get('uses') or {}).get(opener, 0) == 0)
        row = {
            'class': cls,
            'levels': levels,
            'mob': mob,
            'opener': opener,
            'count': len(items),
            'killed': len(killed),
            'died': died,
            'death_rate': round(died / len(items), 3),
            'predicted_death': round(predicted_death, 3),
            'time': actual_time,
            'time_fight': fight_time,
            'time_model': model_time,
            'time_ratio': round(ratio(actual_time, fight_time), 2) if ratio(actual_time, fight_time) else None,
            'taken': actual_taken,
            'taken_fight': fight_taken,
            'taken_ratio': round(ratio(actual_taken, fight_taken), 2) if ratio(actual_taken, fight_taken) else None,
            'casts': actual_casts,
            'casts_model': model_casts,
            'swings': actual_swings,
            'swings_model': model_swings,
            'opener_unused': opener_unused,
        }
        flags = []
        if len(items) >= minimum:
            if row['time_ratio'] is not None and abs(row['time_ratio'] - 1) > tolerance:
                flags.append('time')
            if row['taken_ratio'] is not None and abs(row['taken_ratio'] - 1) > tolerance:
                flags.append('taken')
            if died > 0 and row['death_rate'] > max(2 * predicted_death, predicted_death + 0.05):
                flags.append('deaths')
            if actual_casts is not None and model_casts is not None and model_casts > 0 and \
                    abs(actual_casts - model_casts) / model_casts > tolerance:
                flags.append('casts')
            if killed and opener_unused / len(killed) > 0.5:
                flags.append('opener_unused')
        row['flags'] = flags
        rows.append(row)
    rows.sort(key=lambda row: (-len(row['flags']), -row['count']))
    return rows


def summarize_grounds(grounds, fights):
    by_bot = collections.defaultdict(list)
    for fight in fights:
        by_bot[fight.get('bot')].append(fight)
    stays = collections.defaultdict(list)
    for name, items in by_bot.items():
        items.sort(key=lambda item: item['_when'])
    ordered = collections.defaultdict(list)
    for ground in grounds:
        ordered[ground.get('bot')].append(ground)
    for name, items in ordered.items():
        items.sort(key=lambda item: item['_when'])
        for index, ground in enumerate(items):
            start = ground['_when']
            end = items[index + 1]['_when'] if index + 1 < len(items) else None
            inside = [fight for fight in by_bot.get(name, []) if fight['_when'] >= start and (end is None or fight['_when'] < end)
                      and fight.get('map') == ground.get('map')]
            if not inside:
                continue
            last = inside[-1]['_when'] if end is None else end
            seconds = (last - start).total_seconds()
            if seconds <= 0:
                continue
            kills = sum(1 for fight in inside if fight.get('result') == 'KILLED')
            deaths = sum(1 for fight in inside if fight.get('result') == 'DIED')
            exp = sum(max(0, fight['actual'].get('exp') or 0) for fight in inside)
            forecast = ground.get('forecast') or {}
            stays[(ground.get('class'), band(ground.get('level')), ground.get('map'), forecast.get('name'))].append({
                'seconds': seconds,
                'kills': kills,
                'deaths': deaths,
                'exp': exp,
                'forecast': forecast,
            })
    rows = []
    for (cls, levels, map_id, name), items in stays.items():
        seconds = sum(item['seconds'] for item in items)
        kills = sum(item['kills'] for item in items)
        exp = sum(item['exp'] for item in items)
        deaths = sum(item['deaths'] for item in items)
        predicted = median([item['forecast'].get('kills') for item in items])
        predicted_exp = median([item['forecast'].get('exp') for item in items])
        predicted_deaths = median([item['forecast'].get('deaths') for item in items])
        rows.append({
            'class': cls,
            'levels': levels,
            'map': map_id,
            'ground': name,
            'stays': len(items),
            'seconds': round(seconds),
            'kills_per_s': round(kills / seconds, 4),
            'kills_forecast': round(predicted, 4) if predicted is not None else None,
            'kills_ratio': round(ratio(kills / seconds, predicted), 2) if ratio(kills / seconds, predicted) else None,
            'exp_per_s': round(exp / seconds, 1),
            'exp_forecast': round(predicted_exp, 1) if predicted_exp is not None else None,
            'deaths_per_h': round(deaths / seconds * 3600, 2),
            'deaths_forecast_per_h': round(predicted_deaths * 3600, 2) if predicted_deaths is not None else None,
        })
    rows.sort(key=lambda row: -row['seconds'])
    return rows


def summarize_stays(stays):
    groups = collections.defaultdict(list)
    for stay in stays:
        forecast = stay.get('forecast') or {}
        groups[(stay.get('class'), band(stay.get('level')), stay.get('map'), forecast.get('name'))].append(stay)
    rows = []
    for (cls, levels, map_id, name), items in groups.items():
        seconds = sum(item['actual']['time'] for item in items) / 1000
        if seconds <= 0:
            continue
        kills = sum(item['actual']['kills'] for item in items)
        exp = sum(item['actual']['exp'] for item in items)
        taken = sum(item['actual']['taken'] for item in items)
        swings = sum(item['actual']['swings'] for item in items)
        hits = sum(item['actual']['hits'] for item in items)
        deaths = sum(1 for item in items if item['actual'].get('died'))
        results = collections.Counter(item.get('result') for item in items)
        predicted = median([(item.get('forecast') or {}).get('kills') for item in items])
        predicted_exp = median([(item.get('forecast') or {}).get('exp') for item in items])
        predicted_deaths = median([(item.get('forecast') or {}).get('deaths') for item in items])
        rows.append({
            'class': cls,
            'levels': levels,
            'map': map_id,
            'ground': name,
            'stays': len(items),
            'results': dict(results),
            'seconds': round(seconds),
            'kills_per_s': round(kills / seconds, 4),
            'kills_forecast': round(predicted, 4) if predicted is not None else None,
            'kills_ratio': round(ratio(kills / seconds, predicted), 2) if ratio(kills / seconds, predicted) else None,
            'exp_per_s': round(exp / seconds, 1),
            'exp_forecast': round(predicted_exp, 1) if predicted_exp is not None else None,
            'taken_per_s': round(taken / seconds, 1),
            'hit_rate': round(hits / swings, 3) if swings else None,
            'deaths_per_h': round(deaths / seconds * 3600, 2),
            'deaths_forecast_per_h': round(predicted_deaths * 3600, 2) if predicted_deaths is not None else None,
        })
    rows.sort(key=lambda row: -row['seconds'])
    return rows


def oscillations(decides, window_seconds, limit):
    by_bot = collections.defaultdict(list)
    for entry in decides:
        by_bot[entry.get('bot')].append(entry)
    pairs = collections.Counter()
    gaps = collections.defaultdict(list)
    worst = []
    for name, items in by_bot.items():
        items.sort(key=lambda item: item['_when'])
        flips = 0
        for first, middle, last in zip(items, items[1:], items[2:]):
            if first.get('picked') == last.get('picked') and middle.get('picked') != first.get('picked') and \
                    (last['_when'] - first['_when']).total_seconds() <= window_seconds:
                flips += 1
                pair = f"{first.get('picked')}<->{middle.get('picked')}"
                pairs[pair] += 1
                values = middle.get('values') or {}
                a = values.get(middle.get('picked'))
                b = values.get(first.get('picked'))
                if a is not None and b is not None:
                    gaps[pair].append(abs(a - b) / max(abs(a), abs(b), 1e-9))
        if flips:
            worst.append({'bot': name, 'flips': flips, 'transitions': len(items)})
    worst.sort(key=lambda item: -item['flips'])
    return {
        'pairs': [{'pair': pair, 'flips': count, 'median_relative_gap': round(statistics.median(gaps[pair]), 4)
                   if gaps[pair] else None} for pair, count in pairs.most_common(limit)],
        'bots': worst[:limit],
    }


def report(folder, since_minutes=None, minimum=10, tolerance=0.5, window_seconds=30, limit=20):
    fights = list(records(folder, since_minutes, {'fight'}))
    grounds = list(records(folder, since_minutes, {'ground'}))
    decides = list(records(folder, since_minutes, {'decide'}))
    stays = list(records(folder, since_minutes, {'stay'}))
    fight_rows = summarize_fights(fights, minimum, tolerance)
    return {
        'stay_groups': summarize_stays(stays)[:limit],
        'fights': len(fights),
        'grounds': len(grounds),
        'decides': len(decides),
        'fight_groups': fight_rows[:limit],
        'flagged': sum(1 for row in fight_rows if row['flags']),
        'ground_groups': summarize_grounds(grounds, fights)[:limit],
        'oscillation': oscillations(decides, window_seconds, limit),
    }


def samples(folder, since_minutes=None, cls=None, mob=None, opener=None, limit=5):
    found = []
    for fight in records(folder, since_minutes, {'fight'}):
        forecast = fight.get('forecast') or {}
        if cls and fight.get('class') != cls:
            continue
        if mob and fight.get('mob') != mob:
            continue
        if opener and forecast.get('key') != opener:
            continue
        found.append({key: value for key, value in fight.items() if key != '_when'})
    return found[-limit:]
