import json

parsed = {}
data = None
with open('map-temp.json', 'r', encoding='UTF8') as f:
    data = json.load(f)
    for x in data:
        parsed[x] = data[x]['host']
        del data[x]['host']

with open('host-temp.json', 'w', encoding='UTF8') as f:
    f.write(json.dumps(parsed, indent=4))

with open('map-temp.json', 'w', encoding='utf8') as f:
    f.write(json.dumps(data, indent=4, ensure_ascii=False))
