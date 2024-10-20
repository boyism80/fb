import json

with open('source.txt', 'r', encoding='utf8') as f1:
    with open('result.txt', 'w', encoding='utf8') as f2:
        for line in f1.readlines():
            f2.write(f"{int(line) + 0x7FFF}\n")

# with open('source.txt', 'r', encoding='utf8') as f1:
#     with open('result.txt', 'w', encoding='utf8') as f2:
#         lines = f1.readlines()
#         for line in lines:
#             line = line.strip()
#             if not line:
#                 f2.write('\n')
#                 continue

#             wm_index, p_index = find_world_point(line)
#             f2.write(f"world({wm_index}, {p_index})\n")


            

# with open('result.txt', 'w', encoding='utf8') as f:
    # wm_index = 0
    # for wm, arr in worlds.items():
    #     f.write(f"{wm_index}\t{wm}\n")
    #     wm_index = wm_index + 1

    #     g_index = 0
    #     p_index = 0
    #     for points in arr:
    #         for name, point in points.items():
    #             dst = point['destination']
    #             f.write(f"\t\t{p_index}\t{point['name']}\t{g_index}\t{point['position']['x']}, {point['position']['y']}\t{map_id(dst['map'])}\t{dst['x']}, {dst['y']}\n")
    #             p_index = p_index + 1
    #         g_index = g_index + 1


        # for i in range(len(arr)):
        #     world_group_id = f'{wm}.{i}'
        #     f.write(f'{world_group_id}\n')
        #     for id, world_group in arr[i].items():
        #         name = world_group['name']
        #         map = map_id(world_group['destination']['map'])
        #         if not map:
        #             raise Exception()
                
        #         x = world_group['destination']['x']
        #         y = world_group['destination']['y']
        #         world_x = world_group['position']['x']
        #         world_y = world_group['position']['y']

        #         f.write(f"\t{id}\t{name}\t{map}\t{x}\t{y}\t{world_x}\t{world_y}\n")



# with open('result.txt', 'w', encoding='utf8') as f1:

#     for id, item in items.items():
#         type = item['type']
#         if type != 'weapon':
#             continue

#         equipment_option = item['equipment option'] if 'equipment option' else None
#         if not equipment_option:
#             print(item['name'])
#             continue

#         damage_range = equipment_option['damage range'] if 'damage range' in equipment_option else None
#         if not damage_range:
#             print(item['name'])
#             continue

#         damage_range_s = damage_range['small'] if 'small' in damage_range else None
#         damage_s_min = damage_range_s['min'] if damage_range_s else 0
#         damage_s_max = damage_range_s['max'] if damage_range_s else 0

#         damage_range_l = damage_range['large'] if 'large' in damage_range else None
#         damage_l_min = damage_range_l['min'] if damage_range_l else 0
#         damage_l_max = damage_range_l['max'] if damage_range_l else 0

#         f1.write(f"{damage_s_min} ~ {damage_s_max}\t{damage_l_min} ~ {damage_l_max}\n")
#     # for id, map in maps.items():
#     #     effect = map['effect'].upper() if 'effect' in map else 'NONE'
#     #     f.write(f'{effect}\n')
            

#     # for map_name, arr in warps.items():
#     #     mapid = map_id(map_name)
#     #     f.write(f"{mapid}\n")

#     #     for a in arr:
#     #         x = a['before']['x']
#     #         y = a['before']['y']
#     #         world = a['world'] if 'world' in a else ''
#     #         next_id = map_id(a['map']) if 'map' in a else ''
#     #         next_x = a['after']['x'] if 'after' in a else '0'
#     #         next_y = a['after']['y'] if 'after' in a else '0'
#     #         min_lv = a['limit']['min'] if 'limit' in a else 'null'
#     #         if min_lv == 0:
#     #             min_lv = 'null'

#     #         max_lv = a['limit']['max'] if 'limit' in a else 'null'
#     #         if max_lv == 99:
#     #             max_lv = 'null'
            
#     #         if min_lv == 'null' and max_lv == 'null':
#     #             dsl = ''
#     #         else:
#     #             dsl = f"level({min_lv}, {max_lv})"
#     #         f.write(f'\t{x}\t{y}\t{world}\t{next_id}\t{next_x}\t{next_y}\t{dsl}\n')

#     # for id, spell in spells.items():
#     #     message = spell['message']
#     #     if message == '-':
#     #         message = ''
        
#     #     name = spell['name']
#     #     type = spell['type']
#     #     cast = spell['cast'] if 'cast' in spell else ''
#     #     uncast = spell['uncast'] if 'uncast' in spell else ''
#     #     concast = spell['concast'] if 'concast' in spell else ''
#     #     f.write(f"{id}\t{name}\t{type}\t{cast}\t{uncast}\t{concast}\t{message}\n")

#     # spawns = {}
#     # for spawn in npc_spawns:
#     #     mapid = map_id(spawn['map'])
#     #     direction = spawn['direction'].upper()
#     #     npcid = npc_id(spawn['npc'])
#     #     x = spawn['position']['x']
#     #     y = spawn['position']['y']

#     #     if mapid not in spawns:
#     #         spawns[mapid] = []
#     #     spawns[mapid].append({'map': mapid, 'npc': npcid, 'direction': direction, 'x': x, 'y': y})

#     # for map, spawn in spawns.items():
#     #     f.write(f"{map}\t\t\t\t\n")
#     #     for s in spawn:
#     #         f.write(f"\t{s['npc']}\t{s['x']}\t{s['y']}\t{s['direction']}\n")

#     # for id, npc in npcs.items():
#     #     script = npc['script'] if 'script' in npc else ''
#     #     sell = npc['sell'] if 'sell' in npc else ''
#     #     buy = npc['buy'] if 'buy' in npc else ''
#     #     repair = str(npc['repair'] if 'repair' in npc else 'false').upper()
#     #     hold_money = str(npc['hold_money'] if 'hold_money' in npc else 'false').upper()
#     #     hold_item = str(npc['hold_item'] if 'hold_item' in npc else 'false').upper()
#     #     rename = str(npc['rename'] if 'rename' in npc else 'false').upper()
#     #     f.write(f"{id}\t{npc['name']}\t{npc['look']}\t{npc['color']}\t{script}\t{sell}\t{buy}\t{repair}\t{hold_money}\t{hold_item}\t{rename}\n")

#     # for map_name, spawn in mob_spawns.items():
#     #     mid = map_id(map_name)
#     #     if not mid:
#     #         continue

#     #     f.write(f"{mid}\t\t\t\t\t\t\t\n")
#     #     for s in spawn:
#     #         x0 = s['area']['x0']
#     #         x1 = s['area']['x1']
#     #         y0 = s['area']['y0']
#     #         y1 = s['area']['y1']
#     #         count = s['count']
#     #         mob = mob_id(s['name'])
#     #         rezen = timespan_from_seconds(s['rezen time'])
#     #         begin = f'{x0}, {y0}'
#     #         end = f'{x1}, {y1}'
#     #         f.write(f"\t{x0}\t{x1}\t{y0}\t{y1}\t{count}\t{mob}\t{rezen}\t{begin}\t{end}\n")


#     # result = []
#     # for x in data:
#     #     dsl = {}
#     #     for r in x['require']:
#     #         name = r['item']
#     #         count = r['count']
#     #         if name in dsl:
#     #             dsl[name] = dsl[name] + count
#     #         else:
#     #             dsl[name] = count
        
#     #     dsl_text = []
#     #     for name, count in dsl.items():
#     #         dsl_text.append(f"item({item_id(name)}, {count})")
        
#     #     text = '\n'.join(dsl_text)
#     #     if len(x['require']) > 1:
#     #         text = f"\"{text}\""
        
#     #     result.append(text)
#     # for line in result:
#     #     f.write(line + '\n')