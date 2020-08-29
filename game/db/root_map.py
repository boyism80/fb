#-*- coding:utf-8 -*-

import json
import os
import re

multi_config = {
	'game0': ['낙랑', '초보말하기의길', '가상계'],
	'game1': ['부여', '동부여', '신선선후', '경마장'], # 부여
	'game2': ['고구려', '국내', '평양', '졸본', '개주', '용성', '신성', '후연', '산해', '집안', '선양', '숙군', '신선진선'], # 국내
	'game3': ['도삭산'], #도삭산
	'game4': ['북방', '북천황'], # 북방
	'game5': ['용궁', '.*장군굴.*', '.*장군의굴.*', '청의태자의동굴', '용왕의방'], # 용궁
	'game6': ['일본', '동쪽해안가', '고래갑판'], # 일본,
	'game7': ['만리장성', '장안', '낙양', '복건', '광동', '강서', '현도', '대방', '길림', '상해', '암흑왕', '반고', '소림사', '소호', '운랑'], # 중국,
	'game8': ['극지', '북극'], # 극지방,
	'game9': ['12지신', '.*고개', '.*팔괘방', '청룡', '주작', '백호', '현무', '천상계', '천성현', '수룡,화룡의방', '용왕다람쥐방'],
	'game10': ['고균', '가릉', '폭염'], # 고균도
	'default': ['.*']
}

def find_host(parent_map_name):
	for host in multi_config:
		for regex in multi_config[host]:
			if not regex.startswith('.*'):
				regex = f'{regex}.*'

			if re.search(regex, parent_map_name):
				return host

	return None

def convert(source, dest):
	maps = None
	with open(source, 'r', encoding='utf-8') as f:
		maps = json.load(f)
		first = maps['0']
		for id in maps:
			try:
				parent_id = str(maps[id]['parent'])
				parent_map = maps[parent_id]
				
				host = find_host(parent_map['name'])
				maps[id]['host'] = host
			except Exception as e:
				print(f'cannot find parent map : {maps[id]}')

	with open(dest, 'w', encoding='utf-8') as f:
		f.write(json.dumps(maps, indent=4, ensure_ascii=False))

convert('map - 복사본.json', 'maps_convert.json')