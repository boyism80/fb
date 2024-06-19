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

with open('source.txt', 'r', encoding='utf8') as f:
    lines = f.readlines()

    result = []
    for line in lines:
        split = line.split('\t')
        if not split[0].isdigit():
            result[len(result)-1] = f"{result[len(result)-1]}\n{line}"
        else:
            result.append(line)
    
    item_set = {}
    for line in result:
        item_type = line.split('\t')[7]
        if item_type not in item_set:
            item_set[item_type] = []
        item_set[item_type].append(line)

    for type, lines in item_set.items():
        with open(f'result.{type}.txt', 'w', encoding='utf8') as f:
            f.write('\n'.join(lines))

with open('result.txt', 'w', encoding='utf8') as f:
    for id, item in items.items():
        name = item['name']
        look = item['icon']
        color = item['color']
        price = item['price']
        deposit_price = item['deposit_price']
        trade = item['trade']
        type = item['type'].upper()
        desc = item['desc'] if 'desc' in item else ''
        death_penalty = item['death_penalty'].upper() if 'death_penalty' in item else 'NONE'
        capacity = item['capacity'] if 'capacity' in item else 1
        script_active = item['script']['active'] if 'script' in item and 'active' in item['script'] else ''
        condition = ''
        if 'condition' in item:
            if 'level' in item['condition'] and item['condition']['level'] > 1:
                condition = f"{condition}\nlevel({item['condition']['level']})"

            if 'strength' in item['condition']:
                condition = f"{condition}\nstrength({item['condition']['strength']})"

            if 'dexteritry' in item['condition']:
                condition = f"{condition}\ndexteritry({item['condition']['dexteritry']})"

            if 'intelligence' in item['condition']:
                condition = f"{condition}\nintelligence({item['condition']['intelligence']})"

            if 'cls' in item['condition']:
                cls = item['condition']['cls']
                if cls == 0:
                    cls = 'NONE'
                if cls == 1:
                    cls = 'WARRIOR'
                if cls == 2:
                    cls = 'THIEF'
                if cls == 3:
                    cls = 'MAGICION'
                if cls == 4:
                    cls = 'ASCETIC'
                condition = f"{condition}\ncls({cls})"

            if 'promotion' in item['condition']:
                condition = f"{condition}\npromotion({item['condition']['promotion']})"

            if 'sex' in item['condition']:
                condition = f"{condition}\nsex({item['condition']['sex'].upper()})"
        
        if condition.startswith('\n'):
            condition = condition[1:]

        if '\n' in condition:
            condition = f"\"{condition}\""
        
        dress = item['equipment option']['dress'] if 'eqipment option' in item and item['equipment option']['dress'] else 0
        durability = item['equipment option']['durability'] if 'eqipment option' in item and item['equipment option']['durability'] else 0
        repair = item['equipment option']['repair'] if 'eqipment option' in item and item['equipment option']['repair'] else ''
        dress_script = item['equipment option']['dress_script'] if 'eqipment option' in item and item['equipment option']['dress_script'] else ''
        undress_script = item['equipment option']['undress_script'] if 'eqipment option' in item and item['equipment option']['undress_script'] else ''
        hit = item['equipment option']['hit'] if 'eqipment option' in item and item['equipment option']['hit'] else 0
        damage = item['equipment option']['damage'] if 'eqipment option' in item and item['equipment option']['damage'] else 0
        strength = item['equipment option']['strength'] if 'eqipment option' in item and item['equipment option']['strength'] else 0
        intelligence = item['equipment option']['intelligence'] if 'eqipment option' in item and item['equipment option']['intelligence'] else 0
        dexteritry = item['equipment option']['dexteritry'] if 'eqipment option' in item and item['equipment option']['dexteritry'] else 0
        base_hp = item['equipment option']['base_hp'] if 'eqipment option' in item and item['equipment option']['base_hp'] else 0
        base_mp = item['equipment option']['base_mp'] if 'eqipment option' in item and item['equipment option']['base_mp'] else 0
        hp_percentage = item['equipment option']['hp_percentage'] if 'eqipment option' in item and item['equipment option']['hp_percentage'] else 0.0
        mp_percentage = item['equipment option']['mp_percentage'] if 'eqipment option' in item and item['equipment option']['mp_percentage'] else 0.0
        healing_cycle = item['equipment option']['healing cycle'] if 'eqipment option' in item and item['equipment option']['healing cycle'] else 0
        defensive_physical = item['equipment option']['defensive']['physical'] if 'equipment option' in item and 'defensive' in item['equipment option'] and 'physical' in item['equipment option']['defensive'] else 0
        defensive_magical = item['equipment option']['defensive']['magical'] if 'equipment option' in item and 'defensive' in item['equipment option'] and 'magical' in item['equipment option']['defensive'] else 0

        damage_s_min = item['equipment option']['damage range']['small']['min'] if 'equipment option' in item and 'damage range' in item['equipment option'] and 'small' in item['equipment option']['damage range'] and 'min' in item['equipment option']['damage range']['small'] else 0
        damage_s_max = item['equipment option']['damage range']['small']['max'] if 'equipment option' in item and 'damage range' in item['equipment option'] and 'small' in item['equipment option']['damage range'] and 'max' in item['equipment option']['damage range']['small'] else 0
        damage_l_min = item['equipment option']['damage range']['large']['min'] if 'equipment option' in item and 'damage range' in item['equipment option'] and 'large' in item['equipment option']['damage range'] and 'min' in item['equipment option']['damage range']['large'] else 0
        damage_l_max = item['equipment option']['damage range']['large']['max'] if 'equipment option' in item and 'damage range' in item['equipment option'] and 'large' in item['equipment option']['damage range'] and 'max' in item['equipment option']['damage range']['large'] else 0
        sound = item['equipment option']['sound'] if 'equipment option' in item and 'sound' in item['equipment option'] else 0
        spell = item['equipment option']['spell'] if 'equipment option' in item and 'spell' in item['equipment option'] else ''
        rename = item['equipment option']['rename'] if 'equipment option' in item and 'rename' in item['equipment option'] else ''

        bundle = item['bundle_type'].upper() if 'bundle_type' in item else ''


        if type == 'CONSUME' and bundle == 'BUNDLE':
            f.write(f"{id}\t{name}\t{look}\t{color}\t{price}\t{deposit_price}\t{trade}\t{type}\t{desc}\t{death_penalty}\t{capacity}\t{script_active}\t{condition}\n")
            # f.write(f"{id}\t{name}\t{look}\t{color}\t{price}\t{deposit_price}\t{trade}\t{type}\t{desc}\t{death_penalty}\t{capacity}\t{script_active}\t{condition}\t{dress}\t{durability}\t{repair}\t{dress_script}\t{undress_script}\t{hit}\t{damage}\t{strength}\t{intelligence}\t{dexteritry}\t{base_hp}\t{base_mp}\t{hp_percentage}\t{mp_percentage}\t{healing_cycle}\t{defensive_physical}\t{defensive_magical}\t{damage_s_min}\t{damage_s_max}\t{damage_l_min}\t{damage_l_max}\t{sound}\t{spell}\t{rename}\n")


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
            

    # for map_name, arr in warps.items():
    #     mapid = map_id(map_name)
    #     f.write(f"{mapid}\n")

    #     for a in arr:
    #         x = a['before']['x']
    #         y = a['before']['y']
    #         world = a['world'] if 'world' in a else ''
    #         next_id = map_id(a['map']) if 'map' in a else ''
    #         next_x = a['after']['x'] if 'after' in a else '0'
    #         next_y = a['after']['y'] if 'after' in a else '0'
    #         min_lv = a['limit']['min'] if 'limit' in a else 'null'
    #         if min_lv == 0:
    #             min_lv = 'null'

    #         max_lv = a['limit']['max'] if 'limit' in a else 'null'
    #         if max_lv == 99:
    #             max_lv = 'null'
            
    #         if min_lv == 'null' and max_lv == 'null':
    #             dsl = ''
    #         else:
    #             dsl = f"level({min_lv}, {max_lv})"
    #         f.write(f'\t{x}\t{y}\t{world}\t{next_id}\t{next_x}\t{next_y}\t{dsl}\n')

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