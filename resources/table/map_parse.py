import json
import pathlib
import shutil

def extract_host():
    parsed = {}
    data = None
    with open('map.json', 'r', encoding='UTF8') as f:
        data = json.load(f)
        for x in data:
            parsed[x] = data[x]['host']

    with open('host.json', 'w', encoding='UTF8') as f:
        f.write(json.dumps(parsed, indent=4))

    with open('map.json', 'w', encoding='utf8') as f:
        f.write(json.dumps(data, indent=4, ensure_ascii=False))


def extract_maps():
    data = None
    with open('map.json', 'r', encoding='utf8') as f:
        data = json.load(f)

    hosts = None
    with open('../../internal/table/host.json', 'r', encoding='utf8') as f:
        hosts = json.load(f)

    reverse = {}
    for id in data:
        name = data[id]['name']
        reverse[name] = data[id]
        reverse[name]['id'] = id

    maps = {}
    for id, host in hosts.items():
        if not host in maps:
            maps[host] = {}

        maps[host][id] = data[id]

    # for host, values in maps.items():
    #     pathlib.Path(host).mkdir(parents=True, exist_ok=True)
    #     with open(f'{host}/map.json', 'w', encoding='utf8') as f:
    #         f.write(json.dumps(values, indent=4, ensure_ascii=False))

    for id in data:
        name = data[id]['name']
        reverse[name]['host'] = hosts[id]

    return reverse

def extract_warps(maps):
    data = None
    with open('warp.json', 'r', encoding='utf8') as f:
        data = json.load(f)

    warps = {}
    for name in maps:
        host = maps[name]['host']
        id = maps[name]['id']

        if not host in warps:
            warps[host] = {}

        if name in data:
            if id not in warps[host]:
                warps[host][id] = []

            for i in range(len(data[name])):
                next = data[name][i]['map']
                data[name][i]['to'] = int(maps[next]['id'])
                del data[name][i]['map']
            warps[host][id] += data[name]

    for host in warps:
        pathlib.Path(host).mkdir(parents=True, exist_ok=True)
        with open(f'{host}/warp.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(warps[host], indent=4, ensure_ascii=False))

def extract_npc_spawns(maps):
    npcs = None
    with open('npc.json', 'r', encoding='utf8') as f:
        npcs = json.load(f)
    reverse = {}
    for npc_id, npc in npcs.items():
        npc['id'] = npc_id
        reverse[npc['name']] = npc
    
    npcs = reverse
    del reverse

    data = None
    spawns = {}
    with open('npc.spawn.json', 'r', encoding='utf8') as f:
        data = json.load(f)

    for spawn in data:
        map_name = spawn['map']
        map_id = maps[map_name]['id']
        host = maps[map_name]['host']

        if host not in spawns:
            spawns[host] = []

        # spawn에서 npc 이름 수정해야함
        map_name = spawn['map']
        spawn['map'] = int(maps[map_name]['id'])

        npc_name = spawn['npc']
        spawn['npc'] = int(npcs[npc_name]['id'])
        spawns[host].append(spawn)

    for host in spawns:
        pathlib.Path(host).mkdir(parents=True, exist_ok=True)
        with open(f'{host}/npc.spawn.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(spawns[host], indent=4, ensure_ascii=False))

def extract_mob_spawns(maps):
    mobs = None
    with open('mob.json', 'r', encoding='utf8') as f:
        mobs = json.load(f)

    reverse = {}
    for mob_id, mob in mobs.items():
        mob['id'] = mob_id
        reverse[mob['name']] = mob

    mobs = reverse
    del reverse

    data = None
    spawns = {}
    with open('mob.spawn.json', 'r', encoding='utf8') as f:
        data = json.load(f)

    for map_name, spawn_list in data.items():
        map_id = maps[map_name]['id']
        host = maps[map_name]['host']

        if host not in spawns:
            spawns[host] = {}

        for i in range(len(spawn_list)):
            mob_name = spawn_list[i]['name']
            spawn_list[i]['mob'] = int(mobs[mob_name]['id'])
            del spawn_list[i]['name']

        spawns[host][map_id] = spawn_list

    for host in spawns:
        pathlib.Path(host).mkdir(parents=True, exist_ok=True)
        with open(f'{host}/mob.spawn.json', 'w', encoding='utf8') as f:
            f.write(json.dumps(spawns[host], indent=4, ensure_ascii=False))

def copy_map(maps):
    data = {}
    for map_name in maps:
        host = maps[map_name]['host']
        if host not in data:
            data[host] = []

        data[host].append(int(maps[map_name]['id']))

    for host in data:
        pathlib.Path(f'{host}/maps').mkdir(parents=True, exist_ok=True)
        for map_id in data[host]:
            fname = { 'data': f'{map_id:06}.map', 'block': f'{map_id:06}.block'}
            shutil.copyfile(f"../maps/{fname['data']}", f"{host}/maps/{fname['data']}")
            shutil.copyfile(f"../maps/{fname['block']}", f"{host}/maps/{fname['block']}")


# maps = None
# with open('map.json', 'r', encoding='utf8') as f:
#     maps = json.load(f)

# hosts = None
# with open('../../internal/table/host.json') as f:
#     hosts = json.load(f)

# for map_id in hosts:
#     maps[map_id]['host'] = hosts[map_id]

# with open('map.host.json', 'w', encoding='utf8') as f:
#     f.write(json.dumps(maps, indent=4, ensure_ascii=False))


maps = extract_maps()
extract_warps(maps)
extract_npc_spawns(maps)
extract_mob_spawns(maps)
copy_map(maps)
# extract_host()