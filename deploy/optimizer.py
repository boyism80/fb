import os
import json
import zipfile
import shutil
import glob
import pathlib
import argparse


def __load_maps(path):
    maps = {}
    with open(path, 'r', encoding='utf8') as f:
        maps = json.load(f)

    reverse = {}
    for id in maps:
        name = maps[id]['name']
        reverse[name] = maps[id]
        reverse[name]['id'] = int(id)

    return reverse


def __load_npcs(path):
    npcs = {}
    with open(path, 'r', encoding='utf8') as f:
        npcs = json.load(f)

    reverse = {}
    for id in npcs:
        name = npcs[id]['name']
        reverse[name] = npcs[id]
        reverse[name]['id'] = int(id)

    return reverse


def __load_mobs(path):
    mobs = {}
    with open(path, 'r', encoding='utf8') as f:
        mobs = json.load(f)

    reverse = {}
    for id in mobs:
        name = mobs[id]['name']
        reverse[name] = mobs[id]
        reverse[name]['id'] = int(id)

    return reverse


def __convert_host(maps):
    reverse = { x['id'] : x for x in maps.values() }
    hosts = {}
    for id in reverse:
        hosts[id] = reverse[id]['host group']

    return hosts


def __convert_warps(path, world_path, maps):

    def find_index(worlds, id):
        for wm_i, world in enumerate(worlds.values()):
            for group_i, group in enumerate(world):
                for offset_i, offset_name in enumerate(group.keys()):
                    if offset_name == id:
                        return wm_i, group_i, offset_i

        return None, None, None

    worlds = None
    with open(world_path, 'r', encoding='utf8') as f:
        worlds = json.load(f)

    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    warps = {}
    for name in maps:
        id = maps[name]['id']
        warps[id] = data[name] if name in data else []
    
    for id in warps:
        for warp in warps[id]:
            if 'world' in warp:
                wm, group, offset = find_index(worlds, warp['world'])
                warp['world'] = { 'wm': wm, 'group': group, 'offset': offset }
            else:
                name = warp['map']
                warp['to'] = int(maps[name]['id'])
                del warp['map']

    return warps


def __convert_npc_spawn(path, maps, npcs):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    spawns = {}
    for spawn in data:
        host = maps[spawn['map']]['host group']
        map_id = f"{maps[spawn['map']]['id']:06}"

        if map_id not in spawns:
            spawns[map_id] = []

        spawn['npc'] = int(npcs[spawn['npc']]['id'])
        del spawn['map']
        spawns[map_id].append(spawn)

    return spawns


def __convert_mob_spawn(path, maps, mobs):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    spawns = {}
    for map_name in data:
        map_id = f"{int(maps[map_name]['id']):06}"
        if map_id not in spawns:
            spawns[map_id] = []

        for spawn in data[map_name]:
            spawn['mob'] = mobs[spawn['name']]['id']
            del spawn['name']

        spawns[map_id] += data[map_name]
    
    return spawns


def __convert_world(path, maps):
    worlds = None
    with open(path, 'r', encoding='utf8') as f:
        worlds = json.load(f)

    result = []
    for wname, world in worlds.items():
        cvt_world = []
        for group in world:
            cvt_group = []
            for id, offset in group.items():
                offset['id'] = id
                map_name = offset['destination']['map']
                offset['destination']['map'] = maps[map_name]['id']

                cvt_group.append({
                    'id': id,
                    'name': offset['name'],
                    'position': offset['position'],
                    'destination': {
                        'map': maps[map_name]['id'],
                        'x': offset['destination']['x'],
                        'y': offset['destination']['y']
                    }
                })
            cvt_world.append(cvt_group)
        result.append({'name': wname, 'world': cvt_world})

    return result


def __compress_maps(zfile, path, archive_name):
    for file in [x for x in os.listdir(path) if x.endswith('.map') or x.endswith('.block')]:
        zfile.write(os.path.join(path, file), os.path.join(archive_name, file))

def __compress_table(zfile, path, archive_name):
    for file in [x for x in os.listdir(path) if x.endswith('.json')]:
        zfile.write(os.path.join(path, file), os.path.join(archive_name, file))

def __compress_script(zfile, path, archive_name):
    for (path, dir, files) in os.walk(path):
        for file in files:
            src = os.path.join(path, file)
            dst = os.path.join(*pathlib.Path(path).parts[1:], file)

            zfile.write(src, dst)


def convert(src, dst):
    maps = __load_maps(f'{src}/map.json')
    npcs = __load_npcs(f'{src}/npc.json')
    mobs = __load_mobs(f'{src}/mob.json')

    pathlib.Path(dst).mkdir(parents=True, exist_ok=True)

    warps = __convert_warps(f'{src}/warp.json', f'{src}/world.json', maps)
    with open(os.path.join(dst, 'warp.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(warps, indent=4, ensure_ascii=False, sort_keys=True))

    npc_spawns = __convert_npc_spawn(f'{src}/npc.spawn.json', maps, npcs)
    with open(os.path.join(dst, 'npc.spawn.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(npc_spawns, indent=4, ensure_ascii=False, sort_keys=True))

    mob_spawns = __convert_mob_spawn(f'{src}/mob.spawn.json', maps, mobs)
    with open(os.path.join(dst, 'mob.spawn.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(mob_spawns, indent=4, ensure_ascii=False, sort_keys=True))

    worlds = __convert_world(f'{src}/world.json', maps)
    with open(os.path.join(dst, 'world.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(worlds, indent=4, ensure_ascii=False, sort_keys=True))

    files = [x for x in os.listdir(src) if x.endswith('.json')]
    for file in files:
        from_path = os.path.join(src, file)
        to_path = os.path.join(dst, file)
        if os.path.isfile(to_path):
            continue
        
        shutil.copy(from_path, to_path)

def hosts():
    maps = __load_maps('resources/table/map.json')
    return __convert_host(maps)

def compress(maps, tables, scripts, dst):
    if os.path.isfile(dst):
        os.remove(dst)
    zfile = zipfile.ZipFile(dst, 'w')

    __compress_maps(zfile, maps, 'maps')
    __compress_table(zfile, tables, 'table')
    __compress_script(zfile, scripts, 'scripts')
    zfile.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='fb optimizer')
    parser.add_argument('--id', type=int, default=13, help='game id')
    parser.add_argument('--env', type=str, default='development', help='environment')
    parser.add_argument('--out', type=str, default='game/config.dev.json', help='output file path')
    parser.add_argument('--host', type=str, default='192.168.0.100', help='current host id')
    args = parser.parse_args()

    src = {}
    with open(f'deploy/{args.env}.json', 'r', encoding='utf-8') as f:
        src = json.load(f)


    dst = {}
    with open(args.out, 'r', encoding='utf-8') as f:
        dst = json.load(f)
        dst['id'] = f'game{args.id}'
        dst['ip'] = args.host
        dst['port'] = src['deploy']['game'][f'game{args.id}']['port']
        dst['group'] = src['deploy']['game'][f'game{args.id}']['group']

    with open(args.out, 'w', encoding='utf-8') as f:
        f.write(json.dumps(dst, indent=4, ensure_ascii=True))

    convert('resources/table', 'game/table.dev')