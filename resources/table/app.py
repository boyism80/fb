import json

with open('item.mix.json', 'r', encoding='utf8') as f:
    data = f.read()
    data = json.loads(data)

with open('item.json', 'r', encoding='utf8') as f:
    items = f.read()
    items = json.loads(items)

with open('item.drop.json', 'r', encoding='utf8') as f:
    drops = f.read()
    drops = json.loads(drops)

with open('mob.json', 'r', encoding='utf8') as f:
    mobs = f.read()
    mobs = json.loads(mobs)

with open('mob.spawn.json', 'r', encoding='utf8') as f:
    mob_spawns = f.read()
    mob_spawns = json.loads(mob_spawns)

with open('map.json', 'r', encoding='utf8') as f:
    maps = f.read()
    maps = json.loads(maps)

with open('npc.json', 'r', encoding='utf8') as f:
    npcs = f.read()
    npcs = json.loads(npcs)

with open('npc.spawn.json', 'r', encoding='utf8') as f:
    npc_spawns = f.read()
    npc_spawns = json.loads(npc_spawns)

with open('spell.json', 'r', encoding='utf8') as f:
    spells = f.read()
    spells = json.loads(spells)

with open('warp.json', 'r', encoding='utf8') as f:
    warps = f.read()
    warps = json.loads(warps)

with open('world.json', 'r', encoding='utf8') as f:
    worlds = f.read()
    worlds = json.loads(worlds)

def item_id(name):
    for id, item in items.items():
        if item['name'] == name:
            return id
    
    return None

def mob_id(name):
    for id, mob in mobs.items():
        if mob['name'] == name:
            return id
    
    return None

def map_id(name):
    for id, map in maps.items():
        if map['name'] == name:
            return id
    
    return None

def npc_id(name):
    for id, npc in npcs.items():
        if npc['name'] == name:
            return id
    
    return None

def timespan_from_seconds(seconds):
    minutes = seconds // 60
    seconds = seconds % 60

    hours = minutes // 60
    minutes = minutes % 60

    return f'{hours:02d}:{minutes:02d}:{seconds:02d}'

with open('result.txt', 'w', encoding='utf8') as f:

    # for wm, arr in worlds.items():
    #     for i in range(len(arr)):
    #         world_group_id = f'{wm}.{i}'
    #         f.write(f'{world_group_id}\n')
    #         for id, world_group in arr[i].items():
    #             name = world_group['name']
    #             map = map_id(world_group['destination']['map'])
    #             if not map:
    #                 raise Exception()
                
    #             x = world_group['destination']['x']
    #             y = world_group['destination']['y']
    #             world_x = world_group['position']['x']
    #             world_y = world_group['position']['y']

    #             f.write(f"\t{id}\t{name}\t{map}\t{x}\t{y}\t{world_x}\t{world_y}\n")
            

    for map_name, arr in warps.items():
        mapid = map_id(map_name)
        f.write(f"{mapid}\n")

        for a in arr:
            x = a['before']['x']
            y = a['before']['y']
            world = a['world'] if 'world' in a else ''
            next_id = map_id(a['map']) if 'map' in a else ''
            next_x = a['after']['x'] if 'after' in a else '0'
            next_y = a['after']['y'] if 'after' in a else '0'
            min_lv = a['limit']['min'] if 'limit' in a else 'null'
            if min_lv == 0:
                min_lv = 'null'

            max_lv = a['limit']['max'] if 'limit' in a else 'null'
            if max_lv == 99:
                max_lv = 'null'
            
            if min_lv == 'null' and max_lv == 'null':
                dsl = ''
            else:
                dsl = f"level({min_lv}, {max_lv})"
            f.write(f'\t{x}\t{y}\t{world}\t{next_id}\t{next_x}\t{next_y}\t{dsl}\n')

    # for id, spell in spells.items():
    #     message = spell['message']
    #     if message == '-':
    #         message = ''
        
    #     name = spell['name']
    #     type = spell['type']
    #     cast = spell['cast'] if 'cast' in spell else ''
    #     uncast = spell['uncast'] if 'uncast' in spell else ''
    #     concast = spell['concast'] if 'concast' in spell else ''
    #     f.write(f"{id}\t{name}\t{type}\t{cast}\t{uncast}\t{concast}\t{message}\n")

    # spawns = {}
    # for spawn in npc_spawns:
    #     mapid = map_id(spawn['map'])
    #     direction = spawn['direction'].upper()
    #     npcid = npc_id(spawn['npc'])
    #     x = spawn['position']['x']
    #     y = spawn['position']['y']

    #     if mapid not in spawns:
    #         spawns[mapid] = []
    #     spawns[mapid].append({'map': mapid, 'npc': npcid, 'direction': direction, 'x': x, 'y': y})

    # for map, spawn in spawns.items():
    #     f.write(f"{map}\t\t\t\t\n")
    #     for s in spawn:
    #         f.write(f"\t{s['npc']}\t{s['x']}\t{s['y']}\t{s['direction']}\n")

    # for id, npc in npcs.items():
    #     script = npc['script'] if 'script' in npc else ''
    #     sell = npc['sell'] if 'sell' in npc else ''
    #     buy = npc['buy'] if 'buy' in npc else ''
    #     repair = str(npc['repair'] if 'repair' in npc else 'false').upper()
    #     hold_money = str(npc['hold_money'] if 'hold_money' in npc else 'false').upper()
    #     hold_item = str(npc['hold_item'] if 'hold_item' in npc else 'false').upper()
    #     rename = str(npc['rename'] if 'rename' in npc else 'false').upper()
    #     f.write(f"{id}\t{npc['name']}\t{npc['look']}\t{npc['color']}\t{script}\t{sell}\t{buy}\t{repair}\t{hold_money}\t{hold_item}\t{rename}\n")

    # for map_name, spawn in mob_spawns.items():
    #     mid = map_id(map_name)
    #     if not mid:
    #         continue

    #     f.write(f"{mid}\t\t\t\t\t\t\t\n")
    #     for s in spawn:
    #         x0 = s['area']['x0']
    #         x1 = s['area']['x1']
    #         y0 = s['area']['y0']
    #         y1 = s['area']['y1']
    #         count = s['count']
    #         mob = mob_id(s['name'])
    #         rezen = timespan_from_seconds(s['rezen time'])
    #         f.write(f"\t{x0}\t{x1}\t{y0}\t{y1}\t{count}\t{mob}\t{rezen}\n")


    # result = []
    # for x in data:
    #     dsl = {}
    #     for r in x['require']:
    #         name = r['item']
    #         count = r['count']
    #         if name in dsl:
    #             dsl[name] = dsl[name] + count
    #         else:
    #             dsl[name] = count
        
    #     dsl_text = []
    #     for name, count in dsl.items():
    #         dsl_text.append(f"item({item_id(name)}, {count})")
        
    #     text = '\n'.join(dsl_text)
    #     if len(x['require']) > 1:
    #         text = f"\"{text}\""
        
    #     result.append(text)
    # for line in result:
    #     f.write(line + '\n')