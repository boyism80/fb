import json

with open('json/map.json', 'r', encoding='utf-8') as f:
    maps = json.load(f)

with open('json/warp.json', 'r', encoding='utf-8') as f:
    warps = json.load(f)

roots = {}
with open('json/world.json', 'r', encoding='utf-8') as f:
    data = json.load(f)

    for x1 in data.values():
        for x2 in x1.values():
            id = x2['map']
            if id in roots:
                continue

            roots[id] = maps[str(id)]['name']

del roots[20]
del roots[21]
del roots[856]
del roots[857]
del roots[858]
del roots[859]

queue = []
relations = {}
visit = set()
for id, name in roots.items():
    relations[id] = set()
    queue.append(str(id))

    while queue:
        current = queue.pop(0)
        if current in visit:
            continue

        visit.add(current)
        if current not in warps:
            continue

        for warp in warps[current]:
            if warp['dest']['Type'] != 'map':
                continue

            next_id = warp['dest']['Parameters'][0]
            if next_id in roots:
                continue

            next_id = str(next_id)

            relations[id].add(next_id)
            queue.append(next_id)

for id, map in maps.items():
    del map['host']

with open('json/hosted.txt', 'w', encoding='utf-8') as f:
    for i, map_id in enumerate(relations.keys()):
        buffer = []
        f.write(f"host {i}\n")

        maps[str(map_id)]['host'] = i
        buffer.append(maps[str(map_id)]['name'])
        for ref_map_id in relations[map_id]:
            maps[str(ref_map_id)]['host'] = i
            buffer.append(maps[str(ref_map_id)]['name'])
        
        for line in sorted(buffer):
            f.write(f"\t{line}\n")

with open('json/unhosted.txt', 'w', encoding='utf-8') as f:
    for map in maps.values():
        if 'host' not in map:
            f.write(f"host not set : {map['name']}\n")