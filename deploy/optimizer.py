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

    return hosts


def convert_warps(path, world_path, maps):

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


def convert_npc_spawn(path, maps, npcs):
    data = None
    with open(path, 'r', encoding='utf8') as f:
        data = json.load(f)

    spawns = {}
    for spawn in data:
        host = maps[spawn['map']]['host']
        map_id = f"{maps[spawn['map']]['id']:06}"

        if map_id not in spawns:
            spawns[map_id] = []

        spawn['npc'] = int(npcs[spawn['npc']]['id'])
        del spawn['map']
        spawns[map_id].append(spawn)

    return spawns


def convert_mob_spawn(path, maps, mobs):
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


def convert_world(path, maps):
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


def compress_maps(zfile, path, archive_name):
    for file in [x for x in os.listdir(path) if x.endswith('.map') or x.endswith('.block')]:
        zfile.write(os.path.join(path, file), os.path.join(archive_name, file))

def compress_table(zfile, path, archive_name):
    for file in [x for x in os.listdir(path) if x.endswith('.json')]:
        zfile.write(os.path.join(path, file), os.path.join(archive_name, file))

def compress_script(zfile, path, archive_name):
    for (path, dir, files) in os.walk(path):
        for file in files:
            src = os.path.join(path, file)
            dst = os.path.join(*Path(path).parts[1:], file)

            zfile.write(src, dst)


def resources(src, dst):
    maps = load_maps('resources/table/map.json')
    npcs = load_npcs('resources/table/npc.json')
    mobs = load_mobs('resources/table/mob.json')

    Path(dst).mkdir(parents=True, exist_ok=True)

    warps = convert_warps('resources/table/warp.json', 'resources/table/world.json', maps)
    with open(os.path.join(dst, 'warp.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(warps, indent=4, ensure_ascii=False, sort_keys=True))

    npc_spawns = convert_npc_spawn('resources/table/npc.spawn.json', maps, npcs)
    with open(os.path.join(dst, 'npc.spawn.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(npc_spawns, indent=4, ensure_ascii=False, sort_keys=True))

    mob_spawns = convert_mob_spawn('resources/table/mob.spawn.json', maps, mobs)
    with open(os.path.join(dst, 'mob.spawn.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(mob_spawns, indent=4, ensure_ascii=False, sort_keys=True))

    worlds = convert_world('resources/table/world.json', maps)
    with open(os.path.join(dst, 'world.json'), 'w', encoding='utf8') as f:
        f.write(json.dumps(worlds, indent=4, ensure_ascii=False, sort_keys=True))

    files = [x for x in os.listdir(src) if x.endswith('.json')]
    for file in files:
        from_path = os.path.join(src, file)
        to_path = os.path.join(dst, file)
        if os.path.isfile(to_path):
            continue
        
        shutil.copy(from_path, to_path)

def hosts(dst):
    maps = load_maps('resources/table/map.json')
    hosts = convert_host(maps)

    with open(dst, 'w', encoding='utf8') as f:
        f.write(json.dumps(hosts, indent=4, ensure_ascii=False, sort_keys=True))

def compress(maps, tables, scripts, dst):
    temp = 'temp'
    if os.path.isdir(temp):
        shutil.rmtree(temp)
    os.makedirs(temp, exist_ok=True)
    
    resources(tables, temp)

    if os.path.isfile(dst):
        os.remove(dst)
    zfile = zipfile.ZipFile(dst, 'w')

    compress_maps(zfile, maps, 'maps')
    compress_table(zfile, temp, 'table')
    compress_script(zfile, scripts, 'scripts')
    zfile.close()

    shutil.rmtree(temp)

if __name__ == '__main__':
    compress(maps=os.path.join('resources', 'maps'),
             tables=os.path.join('resources', 'table'),
             scripts=os.path.join('game', 'scripts'),
             dst='resources.zip')