import argparse
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODEL = ROOT / 'include' / 'fb' / 'model' / 'model.h'
SCRIPTS = ROOT / 'server' / 'game' / 'scripts'
OUTPUT = ROOT / 'server' / 'bot' / 'config' / 'load-messages.json'

KINDS = [
    ('EXP_GAINED', [r'^경험치가 .+ 올랐습니다']),
    ('EXP_LOST', [r'^경험치를 .+ 잃었습니다']),
    ('LEVEL_UP', [r'^레벨이 올랐습니다']),
    ('HOLD_FULL', [r'더 이상 가질 수 없습니다', r'더 이상 돈을 가질 수 없습니다', r'소지금이 너무 많습니다', r'돈이 너무 많습니다']),
    ('INVENTORY_FULL', [r'소지품이 꽉 찼습니다', r'소지품이 가득 찼습니다', r'공간이 부족합니다']),
    ('GHOST_FORBIDDEN', [r'귀신은 할 수 없습니다']),
    ('HUMAN_FORBIDDEN', [r'인간은 쓸 수 없음']),
    ('RIDING_FORBIDDEN', [r'말을 타고는 할 수 없습니다']),
    ('DISGUISE_FORBIDDEN', [r'변신 중에는 할 수 없습니다']),
    ('REQUIRE_CLASS', [r'직업을 선택해야 합니다']),
    ('NO_MP', [r'마력이 부족합니다', r'마력이 모자랍니다']),
    ('NO_HP', [r'체력이 부족합니다']),
    ('TARGET_INVALID', [r'대상이 올바르지 않습니다', r'대상이 없습니다', r'대상을 찾을 수 없습니다', r'대상이 보이지 않습니다']),
    ('SPELL_AREA_DENIED', [r'마력이 미치지 않습니다']),
    ('SPELL_DELAY', [r'초 후에 사용할 수 있습니다']),
    ('SPELL_RESISTED', [r'걸리지 않습니다']),
    ('SPELL_ALREADY', [r'이미 걸려있습니다']),
    ('SPELL_FAILED', [r'정신집중에 실패', r'마법이 실패했습니다', r'^%s 실패$']),
    ('SPELL_CAST', [r'외웠습니다', r'걸었습니다', r'가합니다', r'외워주셨습니다']),
    ('MOVE_DENIED', [r'이동할 수 없는 공간', r'들어갈 수 없습니다', r'출두할 수 없습니다', r'그곳으로는 이동할 수 없습니다']),
    ('REVIVE_ACCEPTED', [r'살려줄터이니']),
    ('REVIVE_REFUSED', [r'곱게 빌어도', r'그런 마음가짐으로', r'부활 대답 거부']),
    ('REPAIRED', [r'고치는데 .+전이 들었습니다', r'그냥 고쳐드렸습니다']),
    ('NOT_REPAIRABLE', [r'고칠 물건이 없습니다', r'고칠 수 없습니다', r'이미 고쳐져 있습니다']),
    ('STORE_FULL', [r'더 이상 맡길 수 없습니다', r'더 이상 맡을 수 없습니다', r'맡을 수 없습니다']),
    ('STORED', [r'맡았습니다']),
    ('RETRIEVED', [r'돌려드렸습니다']),
    ('NOT_STORED', [r'맡고 있지 않습니다', r'맡아두고 있지 않습니다', r'맡기지 않았습니다', r'맡아둔 돈이 없습니다', r'맡긴 물건이 없습니다']),
    ('GROUP_NONE', [r'가입된 그룹이 없습니다', r'그룹이 없어용', r'그룹 없음', r'그룹에 참여하지 않았습니다']),
    ('WARP_DENIED', [r'감히 접근할 수 없습니다']),
    ('DROP_WARMTH', [r'죽은 자의 온기']),
    ('EQUIP_BROKEN', [r'깨졌습니다$', r'내구도가 다 닳았습니다']),
    ('EQUIP_DURABILITY', [r'내구도가 .+% 남았습니다']),
    ('EQUIP_DENIED', [r'^레벨이 부족합니다', r'너무 무겁습니다', r'민첩함이 부족합니다', r'지능이 부족합니다', r'착용할 수 없습니다', r'입을 수 없는 물건']),
    ('MONEY_SHORT', [r'돈이 모자랍니다']),
    ('SHOP_NOT_SELL', [r'그런 물건은 안 팝니다']),
    ('SHOP_NOT_BUY', [r'그런 물건은 안 삽니다']),
    ('SHOP_SOLD_TO_ME', [r'전에 팔았습니다']),
    ('SHOP_BOUGHT_FROM_ME', [r'전에 샀습니다']),
    ('COUNT_SHORT', [r'그만큼 가지고 있지 않습니다', r'갯수가 모자라는데요']),
    ('NOT_OWNED', [r'가지고 있지 않', r'가지고 있지도 않']),
    ('ITEM_RECEIVED', [r'님이 .+ 주었습니다$', r'^%s 받았습니다']),
    ('MONEY_RECEIVED', [r'님이 .+전을 주었습니다', r'^금전 .+전 받았습니다']),
    ('GROUP_JOINED', [r'그룹에 참여했습니다', r'님 그룹에 참여']),
    ('GROUP_LEFT', [r'그룹 탈퇴', r'그룹에서 추방당했습니다', r'그룹 해체', r'님이 그룹에서']),
    ('GROUP_TARGET_DISABLED', [r'상대방이 그룹 거부 상태']),
    ('GROUP_DISABLED', [r'^그룹 거부 상태']),
    ('GROUP_SELF', [r'자기 자신과는 그룹할 수 없습니다']),
    ('GROUP_DENIED', [r'자리가 없습니다', r'이미 그룹 참여', r'그룹장만 할 수 있습니다']),
    ('TRADE_DONE', [r'교환에 성공했습니다']),
    ('TRADE_FAILED', [r'교환에 실패했습니다', r'교환을 취소했습니다', r'교환 거부 상태', r'이미 교환 중', r'너무 멀리 떨어져']),
    ('SERVER_NOT_READY', [r'비바람이 휘몰아치고']),
    ('WHISPER_DENIED', [r'귓속말 거부 상태', r'귓속말을 할 수 없는 지역', r'바람의나라에 없습니다']),
]


def pattern_of(text, placeholder):
    parts = re.split(placeholder, text)
    if len(parts) == 1:
        return None
    return '^' + '(.+?)'.join(re.sub(r'([.^$*+?()\[\]{}|\\])', r'\\\1', part) for part in parts) + '$'


def kind_of(text):
    for kind, rules in KINDS:
        if any(re.search(rule, text) for rule in rules):
            return kind
    return ''


def collect_constants():
    source = MODEL.read_text(encoding='utf-8')
    for name, text in re.findall(r'(MESSAGE_[A-Z0-9_]+) = "([^"]*)"', source):
        yield 'const', name, text, pattern_of(text, r'\{\}')


def collect_scripts():
    literal = re.compile(r"(me|npc|npc_obj|you|ch|target|caster):(message|chat)\(\s*(string\.format\(\s*)?'([^']+)'")
    for path in SCRIPTS.rglob('*.lua'):
        if 'test' in path.parts or path.name == 'script.lua':
            continue
        source = path.read_text(encoding='utf-8', errors='ignore')
        for owner, call, formatted, text in literal.findall(source):
            channel = 'npc' if call == 'chat' else 'lua'
            pattern = pattern_of(text, r'%[sd]') if formatted else None
            yield channel, str(path.relative_to(ROOT)).replace('\\', '/'), text, pattern


def build():
    entries = {}
    for channel, origin, text, pattern in list(collect_constants()) + list(collect_scripts()):
        key = pattern or text
        entry = entries.setdefault(key, {'kind': kind_of(text), 'text': text, 'channels': [], 'origins': []})
        if pattern is not None:
            entry['pattern'] = pattern
        if channel not in entry['channels']:
            entry['channels'].append(channel)
        if origin not in entry['origins'] and len(entry['origins']) < 5:
            entry['origins'].append(origin)
    return sorted(entries.values(), key=lambda entry: (entry['kind'] == '', entry['kind'], entry['text']))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', default=str(OUTPUT))
    parser.add_argument('--review', default=None)
    args = parser.parse_args()

    entries = build()
    pathlib.Path(args.output).write_text(json.dumps({'messages': entries}, ensure_ascii=False, indent=1), encoding='utf-8')

    classified = [entry for entry in entries if entry['kind']]
    print(f'messages={len(entries)} classified={len(classified)} kinds={len({entry["kind"] for entry in classified})}')

    if args.review:
        lines = ['| 분류 | 문구 | 경로 |', '|------|------|------|']
        for entry in classified:
            lines.append(f"| {entry['kind']} | {entry['text']} | {', '.join(entry['channels'])} |")
        pathlib.Path(args.review).write_text('\n'.join(lines) + '\n', encoding='utf-8')


if __name__ == '__main__':
    main()
