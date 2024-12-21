# -*- coding:utf-8 -*-

import json

def name2map(name):
    for id, map in maps.items():
        if map['name'] == name:
            return map
    return None

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

additional_roots = [
    name2map('가상계')['id'],
    25383,
    1836,
    15183,
    1948,
    2412,
    name2map('청룡의심장')['id'],
    name2map('풍곡입구')['id'],
    name2map('지옥')['id'],
    name2map('위례성')['id']
]

links = {}
links[name2map('가상계')['id']] = [
    name2map('낙랑의방')['id'],
    name2map('세계이동')['id'],
    name2map('불량이용자 제재')['id'],
]

links[name2map('12지신의유적')['id']] = [
    name2map('용왕용마의침소')['id'],
    name2map('용왕용천의침소')['id'],
    name2map('용왕용랑의침소')['id'],
    name2map('용왕용겸의침소')['id'],
    name2map('용왕용섬의침소')['id'],
    name2map('천상계제이계5')['id'],
    name2map('구명곡1')['id']
]

links[name2map('세시마을')['id']] = [
    name2map('세시논1-1')['id'],
    name2map('세시논1-2')['id'],
    name2map('세시논1-3')['id'],
    name2map('세시논1-4')['id'],
    name2map('세시논1-5')['id']
]

links[name2map('해골왕의방')['id']] = [
    name2map('파괴왕의제단')['id']
]

links[name2map('부여성')['id']] = [
    name2map('제곡의방')['id'],
    name2map('부여-일본선착장')['id']
]

links[name2map('위례성')['id']] = [
    name2map('위례성-동쪽숲갈림길')['id'],
]

links[name2map('청룡의심장')['id']] = [
    name2map('백호팔괘방')['id'],
    name2map('청룡팔괘방')['id'],
    name2map('주작팔괘방')['id'],
    name2map('현무팔괘방')['id'],

    name2map('백호성팔괘방')['id'],
    name2map('청룡성팔괘방')['id'],
    name2map('주작성팔괘방')['id'],
    name2map('현무성팔괘방')['id'],

    name2map('주작의심장')['id'],
    name2map('주작의머리')['id'],
    name2map('주작의비밀의방')['id'],
    name2map('주작지하수로')['id'],
    name2map('주작성팔괘방')['id'],
    name2map('주작성갑옷점')['id'],
    name2map('주작성포목상')['id'],
    name2map('주작성푸줏간')['id'],
    name2map('주작성학반령1')['id'],
    name2map('주작성학반령2')['id'],
    name2map('주작성학반령3')['id'],
    name2map('주작성학반령4')['id'],
    name2map('현무의심장')['id'],
    name2map('백호의심장')['id'],
    name2map('백호지하수로')['id'],
    name2map('백호지하수로1층')['id'],
    name2map('청룡지하수로1층')['id'],
    name2map('현무지하수로1층')['id']
]

links[name2map('현도성')['id']] = [
    name2map('도삭산비밀의방')['id'],
    name2map('도삭산1000층갑옷점')['id'],
    name2map('도삭산1000층대장간')['id'],
    name2map('도삭산1000층우성황')['id'],
    name2map('도삭산1000층좌성황')['id'],
    name2map('도삭산1000층주막')['id'],
    name2map('도삭산1000층푸줏간')['id']
]

links[name2map('낙양성')['id']] = [
    name2map('낙양천구굴8')['id'],
]

links[name2map('도삭산')['id']] = [
    name2map('현도후굴1')['id']
]

links[name2map('북방대초원40')['id']] = [
    name2map('북천황의감옥')['id']
]

links[name2map('장안성')['id']] = [
    name2map('진시황릉입구')['id'],
    name2map('소호의방')['id']
]

links[name2map('길림성')['id']] = [
    name2map('길림성빈방2')['id']
]

links[name2map('광동성')['id']] = [
    name2map('순제의방')['id'],
    name2map('반고의방')['id'],
    name2map('반고의신전1')['id']
]

links[name2map('일본신궁')['id']] = [
    name2map('요제의방')['id']
]

links[name2map('국내성')['id']] = [
    name2map('전욱의방')['id'],
    name2map('진원관1')['id'],
    name2map('고구려무한장')['id'],
    name2map('고구려감옥')['id']
]

links[name2map('선비족입구')['id']] = [
    name2map('선비족1-4(4)')['id'],
    name2map('선비족1-4(5)')['id'],
    name2map('선비족1-4(6)')['id'],
    name2map('선비족1-4(7)')['id'],
    name2map('만리장성1')['id'],
    name2map('만리장성2')['id'],
    name2map('만리장성3')['id'],
    name2map('만리장성4')['id']
]

links[name2map('풍곡입구')['id']] = [
    name2map('풍 1-1')['id'],
]

links[name2map('오녀산성')['id']] = [
    name2map('엄호수대기실')['id'],
    name2map('숲속길1-1')['id'],
    name2map('깊은숲1-1')['id'],
]

links[name2map('산적굴1')['id']] = [
    name2map('산적소굴1-1')['id'],
]

links[name2map('지옥')['id']] = [
    name2map('도산지옥6-2')['id'],
    name2map('지옥의갈림길')['id'],
    name2map('한빙비밀의방')['id'],
    name2map('흑암지옥지하입구')['id'],
    name2map('도산지옥')['id'],
    name2map('귀문마동1-1')['id'],
    name2map('귀문마동1-2')['id'],
    name2map('귀문마동1-3')['id'],
    name2map('귀문마동1-4')['id'],
    name2map('귀문마동1-5')['id'],
    name2map('귀문마동1-6')['id'],
    name2map('귀문마동1-7')['id'],
    name2map('귀문마동1-8')['id'],
    name2map('귀문마동1-9')['id'],
    name2map('귀문방주실')['id'],
    name2map('귀문마동1입구')['id'],
    name2map('귀문마동2입구')['id'],
    name2map('귀문마동3입구')['id'],
    name2map('귀문마동4입구')['id'],
    name2map('귀문마동5입구')['id'],
    name2map('귀문마동6입구')['id'],
    name2map('귀문마동7입구')['id'],
    name2map('귀문마동8입구')['id'],
    name2map('귀문마동9입구')['id'],
    name2map('귀문방주실입구')['id'],
    name2map('귀문혈동1-1')['id'],
    name2map('귀문혈동1-2')['id'],
    name2map('귀문혈동1-3')['id'],
    name2map('귀문혈동1-4')['id'],
    name2map('귀문혈동1-5')['id'],
    name2map('귀문혈동1-6')['id'],
    name2map('귀문혈동1-7')['id'],
    name2map('귀문혈동1-8')['id'],
    name2map('귀문혈동1-9')['id'],
    name2map('귀문혈동1-10')['id'],
    name2map('귀문혈동1입구')['id'],
    name2map('귀문혈동2입구')['id'],
    name2map('귀문혈동3입구')['id'],
    name2map('귀문혈동4입구')['id'],
    name2map('귀문혈동5입구')['id'],
    name2map('귀문혈동6입구')['id'],
    name2map('귀문혈동7입구')['id'],
    name2map('귀문혈동8입구')['id'],
    name2map('귀문혈동9입구')['id'],
    name2map('귀문혈동10입구')['id'],
    name2map('귀문동초입1')['id'],
    name2map('귀문동초입2')['id'],
    name2map('귀문동초입3')['id'],
    name2map('귀문동초입4')['id'],
    name2map('귀문동초입5')['id'],
    name2map('귀문동초입6')['id'],
    name2map('귀문동초입7')['id'],
    name2map('귀문동초입8')['id'],
    name2map('귀문동초입9')['id'],
    name2map('귀문동초입10')['id'],
    name2map('귀문진동1-1')['id'],
    name2map('귀문진동1-2')['id'],
    name2map('귀문진동1-3')['id'],
    name2map('귀문진동1-4')['id'],
    name2map('귀문진동1-5')['id'],
    name2map('귀문진동1-6')['id'],
    name2map('귀문진동1-7')['id'],
    name2map('귀문진동1-8')['id'],
    name2map('귀문진동1-9')['id'],
    name2map('귀문진방주실')['id'],
    name2map('귀문진동1입구')['id'],
    name2map('귀문진동2입구')['id'],
    name2map('귀문진동3입구')['id'],
    name2map('귀문진동4입구')['id'],
    name2map('귀문진동5입구')['id'],
    name2map('귀문진동6입구')['id'],
    name2map('귀문진동7입구')['id'],
    name2map('귀문진동8입구')['id'],
    name2map('귀문진동9입구')['id'],
    name2map('귀문진방주실입구')['id'],
    name2map('거해지옥7-10')['id']
]

links[name2map('백두촌')['id']] = [
    name2map('백운봉1')['id'],
    name2map('백운봉2')['id'],
    name2map('백운봉3')['id'],
    name2map('백운봉4')['id'],
    name2map('백운봉5')['id'],
    name2map('백운봉6')['id'],
    name2map('백운봉7')['id'],
    name2map('백운봉8')['id'],
    name2map('백운봉9')['id'],
    name2map('백운봉10')['id'],
    name2map('백두봉1')['id'],
    name2map('백두봉2')['id'],
    name2map('백두봉3')['id'],
    name2map('백두봉4')['id'],
    name2map('백두봉5')['id'],
    name2map('백두봉6')['id'],
    name2map('백두봉7')['id'],
    name2map('백두봉8')['id'],
    name2map('백두봉9')['id'],
    name2map('백두봉10')['id'],
    name2map('월요의동굴1')['id'],
    name2map('화요의동굴1')['id'],
    name2map('수요의동굴1')['id'],
    name2map('목요의동굴1')['id'],
    name2map('금요의동굴1')['id'],
    name2map('토요의동굴1')['id'],
    name2map('일요의동굴1')['id'],
    name2map('팔요의동굴1')['id']
]

for x in additional_roots:
    roots[x] = maps[str(x)]['name']

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
        if current in warps:
            for warp in warps[current]:
                if warp['dest']['Type'] != 'map':
                    continue

                next_id = warp['dest']['Parameters'][0]
                if next_id in roots:
                    continue

                next_id = str(next_id)

                relations[id].add(next_id)
                queue.append(next_id)

        if int(current) in links:
            for link in links[int(current)]:
                relations[id].add(str(link))
                queue.append(str(link))

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