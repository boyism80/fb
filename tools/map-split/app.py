# -*- coding:utf-8 -*-

ENABLED_FORCE_LINK = True
WORKING_DIR = '../../game/json'

import json

def name2map(name):
    for id, map in maps.items():
        if map['name'] == name:
            return map
    return None

def relative_of_word(word):
    for id, map in maps.items():
        if word in map['name']:
            yield map['id']
    return None

with open(f'{WORKING_DIR}/map.json', 'r', encoding='utf-8') as f:
    maps = json.load(f)

with open(f'{WORKING_DIR}/warp.json', 'r', encoding='utf-8') as f:
    warps = json.load(f)

roots = {}
with open(f'{WORKING_DIR}/world.json', 'r', encoding='utf-8') as f:
    data = json.load(f)

    for x1 in data.values():
        for x2 in x1.values():
            id = x2['map']
            if id in roots:
                continue

            roots[id] = maps[str(id)]['name']


roots = [
    name2map('가상계')['id'],
    name2map('국내성')['id'],
    name2map('부여성')['id'],
    name2map('신부여성남서쪽')['id'],
    name2map('12지신의유적')['id'],
    name2map('천상계제일계')['id'],
    name2map('극지방입구')['id'],
    name2map('선비족입구')['id'],
    name2map('대방성')['id'],
    name2map('낙양성')['id'],
    name2map('현도성')['id'],
    name2map('장안성')['id'],
    name2map('흉노족1')['id'],
    name2map('남양1')['id'],
    name2map('남경1')['id'],
    name2map('상해1')['id'],
    name2map('강서성')['id'],
    name2map('광동성')['id'],
    name2map('도삭산')['id'],
    name2map('복건성')['id'],
    name2map('고균도')['id'],
    name2map('고구려평양성')['id'],
    name2map('고구려졸본성')['id'],
    name2map('고구려숙군성')['id'],
    name2map('고구려선양성')['id'],
    name2map('고구려집안성')['id'],
    name2map('고구려산해관')['id'],
    name2map('고구려신성')['id'],
    name2map('고구려용성')['id'],
    name2map('고구려후연성')['id'],
    name2map('고구려개주')['id'],
    name2map('부여범안성')['id'],
    name2map('부여관령성')['id'],
    name2map('부여어양성')['id'],
    name2map('부여상곡성')['id'],
    name2map('부여천안궁성')['id'],
    name2map('부여만번한성')['id'],
    name2map('부여읍루성')['id'],
    name2map('부여장훈성')['id'],
    name2map('부여송원성')['id'],
    name2map('동부여성')['id'],
    name2map('청룡의심장')['id'],
    name2map('풍곡입구')['id'],
    name2map('지옥')['id'],
    name2map('위례성')['id'],
    name2map('백두산관문')['id'],
    name2map('일본신궁')['id']
]

links = {}
links[name2map('가상계')['id']] = [
    name2map('낙랑의방')['id'],
    name2map('세계이동')['id'],
    name2map('불량이용자 제재')['id'],
]

links[name2map('12지신의유적')['id']] = [
    name2map('PK12지신의유적')['id'],
    name2map('용왕토끼방')['id'],
    name2map('용왕용마의침소')['id'],
    name2map('용왕용천의침소')['id'],
    name2map('용왕용랑의침소')['id'],
    name2map('용왕용겸의침소')['id'],
    name2map('용왕용섬의침소')['id'],
    name2map('천상계제이계5')['id'],
    name2map('용왕용신의침소')['id'],
    name2map('구명곡1')['id'],
    *list(relative_of_word('한고개')),
    *list(relative_of_word('두고개')),
    *list(relative_of_word('세고개')),
    *list(relative_of_word('네고개'))
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
    name2map('부여-일본선착장')['id'],
    *list(relative_of_word('부여비밀세작의집10')),
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

    name2map('주작성')['id'],
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
    name2map('요제의방')['id'],
    name2map('닌자의방')['id']
]

links[name2map('문지기의방11')['id']] = [
    name2map('문지기의방12')['id']
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

links[name2map('구명곡8')['id']] = [
    name2map('구명곡9')['id'],
]

links[name2map('오녀산성')['id']] = [
    name2map('엄호수대기실')['id'],
    name2map('숲속길1-1')['id'],
    name2map('깊은숲1-1')['id'],
    name2map('엄호수의유역1')['id'],
]

links[name2map('산적굴1')['id']] = [
    name2map('산적소굴1-1')['id'],
]

links[name2map('고균도')['id']] = [
    name2map('가릉도')['id'],
    name2map('폭염도')['id'],
    name2map('난파선1')['id'],
    name2map('난파선선장의방')['id'],
    *list(relative_of_word('고균도폐허동굴9')),
]

links[name2map('지옥')['id']] = [
    *list(relative_of_word('검수지옥')),
    *list(relative_of_word('발설지옥')),
    *list(relative_of_word('한빙지옥')),
    *list(relative_of_word('독사지옥')),
    *list(relative_of_word('흑암지옥')),
    *list(relative_of_word('풍도지옥')),
    *list(relative_of_word('화탕지옥')),
    *list(relative_of_word('거해지옥')),
    *list(relative_of_word('도산지옥')),
    *list(relative_of_word('귀문')),
    name2map('거짓지옥배')['id'],
    name2map('지옥마왕아수라의방')['id'],
    name2map('지옥의갈림길')['id'],
    name2map('한빙비밀의방')['id'],
    name2map('흑암지옥지하입구')['id'],
]

links[name2map('백두촌')['id']] = [
    *list(relative_of_word('백운봉')),
    *list(relative_of_word('백두봉')),
    *list(relative_of_word('요의동굴1')),
]

queue = []
relations = {}
visit = set()
for id in roots:
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

        if ENABLED_FORCE_LINK:
            if int(current) in links:
                for link in links[int(current)]:
                    relations[id].add(str(link))
                    queue.append(str(link))

for id, map in maps.items():
    del map['host']

with open('hosted.txt', 'w', encoding='utf-8') as f:
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

max_host = i

with open('unhosted.txt', 'w', encoding='utf-8') as f:
    for map in maps.values():
        if 'host' not in map:
            map['host'] = i + 1
            f.write(f"host not set : {map['name']}\n")

with open('result.txt', 'w', encoding='utf-8') as f:
    for map in maps.values():
        t = f"{map['id']}	{map['name']}	{map['root']}	{map['bgm']}	{map['effect']}	{map['host']}\n"
        f.write(t)