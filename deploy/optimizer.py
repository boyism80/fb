import os
import json
import zipfile
import shutil
import glob
from pathlib import Path

def load_maps(path):
    maps = {}
    with open(path, 'r', encoding='utf8') as f:
        maps = json.load(f)

    reverse = {}
    for id in maps:
        name = maps[id]['name']
        reverse[name] = maps[id]
        reverse[name]['id'] = int(id)

    return reverse

def load_npcs(path):
    npcs = {}
    with open(path, 'r', encoding='utf8') as f:
        npcs = json.load(f)

    reverse = {}
    for id in npcs:
        name = npcs[id]['name']
        reverse[name] = npcs[id]
        reverse[name]['id'] = int(id)

    return reverse

def load_mobs(path):
    mobs = {}
    with open(path, 'r', encoding='utf8') as f:
        mobs = json.load(f)

    reverse = {}
    for id in mobs:
        name = mobs[id]['name']
        reverse[name] = mobs[id]
        reverse[name]['id'] = int(id)

    return reverse

def convert_host(maps):
    reverse = { x['id'] : x for x in maps.values() }
    hosts = {}
    for id in reverse:
        hosts[id] = reverse[id]['host']

    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    with open(f'table.deploy/host.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(hosts, indent=4, ensure_ascii=False))

    return hosts


def convert_warps(path, maps):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    warps = {}
    for name in maps:
        host = maps[name]['host']
        id = maps[name]['id']

        if host not in warps:
            warps[host] = {}

        if name in data:
            if id not in warps[host]:
                warps[host][id] = []

            warps[host][id] += data[name]

    for host in warps:
        for id in warps[host]:
            for warp in warps[host][id]:
                name = warp['map']
                warp['to'] = int(maps[name]['id'])
                del warp['map']


    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    for host in warps:
        with open(f'table.deploy/warp.{host}.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(warps[host], indent=4, ensure_ascii=False))

    return warps

def convert_npc_spawn(path, maps, npcs):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    spawns = {}
    for spawn in data:
        host = maps[spawn['map']]['host']

        if host not in spawns:
            spawns[host] = {}

        map_id = f"{maps[spawn['map']]['id']:06}"

        if map_id not in spawns[host]:
            spawns[host][map_id] = []

        spawn['npc'] = int(npcs[spawn['npc']]['id'])
        del spawn['map']
        spawns[host][map_id].append(spawn)

    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    for host in spawns:
        with open(f'table.deploy/npc.spawn.{host}.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(spawns[host], indent=4, ensure_ascii=False))

    return spawns

def convert_mob_spawn(path, maps, mobs):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    spawns = {}
    for map_name in data:
        host = maps[map_name]['host']

        if host not in spawns:
            spawns[host] = {}

        map_id = f"{int(maps[map_name]['id']):06}"
        if map_id not in spawns[host]:
            spawns[host][map_id] = []

        for spawn in data[map_name]:
            spawn['mob'] = mobs[spawn['name']]['id']
            del spawn['name']

        spawns[host][map_id] += data[map_name]

    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    for host in spawns:
        with open(f'table.deploy/mob.spawn.{host}.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(spawns[host], indent=4, ensure_ascii=False))

    return spawns


def convert_world(path, maps):
    worlds = None
    with open(path, 'r', encoding='utf8') as f:
        worlds = json.load(f)

    for groups in worlds.values():
        for group in groups:
            for config in group.values():
                map_name = config['destination']['map']
                config['destination']['map'] = maps[map_name]['id']

    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    with open(f'table.deploy/world.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(worlds, indent=4, ensure_ascii=False))


def compress_maps(maps):
    data = {}
    for name in maps:
        host = maps[name]['host']

        if host not in data:
            data[host] = []

        data[host].append(f"{maps[name]['id']:06}")


    Path('table.deploy').mkdir(parents=True, exist_ok=True)
    for host in data:
        zfile = zipfile.ZipFile(f'table.deploy/maps.{host}.zip', 'w')
        for map_id in data[host]:
            data_name = f'resources/maps/{map_id}.map'
            block_name = f'resources/maps/{map_id}.block'

            zfile.write(data_name, os.path.basename(data_name))
            zfile.write(block_name, os.path.basename(block_name))

        zfile.close()

def resources():
    maps = load_maps('resources/table/map.json')
    npcs = load_npcs('resources/table/npc.json')
    mobs = load_mobs('resources/table/mob.json')

    convert_host(maps)
    convert_warps('resources/table/warp.json', maps)
    convert_npc_spawn('resources/table/npc.spawn.json', maps, npcs)
    convert_mob_spawn('resources/table/mob.spawn.json', maps, mobs)
    convert_world('resources/table/world.json', maps)
    compress_maps(maps)

if __name__ == '__main__':
    resources()

    game_config = None
    with open('game/config.dev.json', 'r', encoding='utf8') as f:
        game_config = json.load(f)

    internal_config = None
    with open('internal/config.dev.json', 'r', encoding='utf8') as f:
        internal_config = json.load(f)

    current_id = game_config['id']
    current_host = internal_config['hosts'][current_id]

    game_config['ip'] = current_host['ip']
    game_config['port'] = current_host['port']
    with open('game/config.dev.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(game_config, indent=4, ensure_ascii=False))

    if os.path.isdir('game/table.dev'):
        shutil.rmtree('game/table.dev')
    Path('game/table.dev').mkdir(parents=True, exist_ok=True)

    if os.path.isdir('game/maps'):
        shutil.rmtree('game/maps')
    Path('game/maps').mkdir(parents=True, exist_ok=True)

    for fname in glob.glob(os.path.join('resources/table', '*.json')):
        shutil.copy(fname, 'game/table.dev')

    if os.path.isfile(f'table.deploy/mob.spawn.{current_id}.json'):
        shutil.copy(f'table.deploy/mob.spawn.{current_id}.json', 'game/table.dev/mob.spawn.json')
    else:
        os.remove('game/table.dev/mob.spawn.json')

    if os.path.isfile(f'table.deploy/npc.spawn.{current_id}.json'):
        shutil.copy(f'table.deploy/npc.spawn.{current_id}.json', 'game/table.dev/npc.spawn.json')
    else:
        os.remove('game/table.dev/npc.spawn.json')

    if os.path.isfile(f'table.deploy/warp.{current_id}.json'):
        shutil.copy(f'table.deploy/warp.{current_id}.json', 'game/table.dev/warp.json')
    else:
        os.remove('game/table.dev/warp.json')

    if os.path.isfile(f'table.deploy/world.json'):
        shutil.copy(f'table.deploy/world.json', 'game/table.dev/world.json')
    else:
        os.remove('game/table.dev/world.json')

    if os.path.isfile(f'table.deploy/maps.{current_id}.zip'):
        shutil.copy(f'table.deploy/maps.{current_id}.zip', 'game/maps/maps.zip')

        zfile = zipfile.ZipFile('game/maps/maps.zip')
        zfile.extractall('game/maps')
        zfile.close()
        os.remove('game/maps/maps.zip')

    shutil.rmtree('table.deploy')