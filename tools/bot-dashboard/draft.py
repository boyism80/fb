import json
import math
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'server' / 'bot' / 'docs' / 'dashboard-draft'
FONTS = 'https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;500;600;700&family=Noto+Sans+KR:wght@400;500;700&display=swap'
ECHARTS = 'https://cdn.jsdelivr.net/npm/echarts@5.5.1/dist/echarts.min.js'

STYLE = """
:root {
  --bg: #0d1117; --card: #161b22; --sunken: #010409; --line: #30363d; --line-soft: #21262d;
  --text: #e6edf3; --muted: #8d96a0; --faint: #6e7681;
  --accent: #4493f8; --accent-soft: rgba(56, 139, 253, 0.10); --accent-line: rgba(56, 139, 253, 0.40);
  --go: #238636; --go-line: rgba(240, 246, 252, 0.10);
  --good: #3fb950; --bad: #f85149; --warn: #d29922; --idle: #30363d; --chip: rgba(110, 118, 129, 0.15);
  --sans: 'Noto Sans KR', 'Malgun Gothic', sans-serif; --mono: 'JetBrains Mono', Consolas, monospace;
  color-scheme: dark;
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--text); font: 13px/1.55 var(--sans); font-variant-numeric: tabular-nums; }
button { font: inherit; }
.bar { height: 50px; border-bottom: 1px solid var(--line); display: flex; align-items: center; gap: 28px; padding: 0 max(16px, 40px); }
.logo { font: 700 13px var(--mono); letter-spacing: .12em; }
.logo b { color: var(--muted); }
.views { display: flex; gap: 18px; font-size: 12px; }
.views a { color: var(--faint); text-decoration: none; cursor: pointer; }
.views a.on { color: var(--text); }
.online { margin-left: auto; font-size: 11.5px; color: var(--muted); display: flex; align-items: center; gap: 7px; }
.online::before { content: ''; width: 6px; height: 6px; border-radius: 50%; background: var(--good); }
.page { max-width: 1000px; margin: 0 auto; padding: 44px max(16px, 24px) 24px; }
.eyebrow { font: 500 9.5px var(--mono); letter-spacing: .16em; color: var(--muted); text-transform: uppercase; }
.title { display: flex; justify-content: space-between; align-items: flex-start; gap: 16px; }
h1 { margin: 6px 0 8px; font-size: 25px; font-weight: 700; letter-spacing: -.01em; }
.lead { margin: 0; color: var(--muted); font-size: 12.5px; }
.status { margin-top: 26px; border: 1px solid var(--line); color: var(--text); background: var(--card); font: 500 10.5px var(--mono); letter-spacing: .08em; padding: 7px 12px; border-radius: 3px; white-space: nowrap; }
.stats { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 12px; margin: 30px 0 16px; }
.card { background: var(--card); border: 1px solid var(--line); border-radius: 4px; min-width: 0; }
.stat { padding: 18px 16px 16px; }
.stat .k { font-size: 11.5px; color: var(--muted); }
.stat .v { font: 700 20px var(--sans); margin: 14px 0 10px; }
.stat .v span { color: var(--faint); font-weight: 400; margin: 0 2px; }
.stat .s { font-size: 10.5px; color: var(--faint); }
.stat .states { display: flex; gap: 10px; flex-wrap: wrap; font: 10.5px var(--mono); }
.stat .states i { display: inline-block; width: 7px; height: 7px; border-radius: 2px; margin-right: 4px; }
.grid { display: grid; grid-template-columns: 250px minmax(0, 1fr); gap: 16px; align-items: start; }
.stack { display: grid; gap: 16px; min-width: 0; }
.head { display: flex; justify-content: space-between; align-items: baseline; gap: 10px; padding: 18px 16px 14px; border-bottom: 1px solid var(--line); }
.head h2 { margin: 0; font-size: 13px; font-weight: 700; }
.head .sub { font-size: 10.5px; color: var(--faint); margin-top: 3px; }
.head .note { font-size: 10.5px; color: var(--faint); white-space: nowrap; }
.form { padding: 16px; display: grid; gap: 14px; }
.two { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
.fld { display: grid; gap: 6px; min-width: 0; }
.fld label { font-size: 11px; color: var(--muted); }
.in { height: 30px; border: 1px solid var(--line); background: var(--sunken); border-radius: 3px; padding: 0 10px; color: var(--text); font: 11.5px var(--sans); display: flex; align-items: center; width: 100%; min-width: 0; white-space: nowrap; overflow: hidden; }
.in.sel::after { content: '⌄'; margin-left: auto; color: var(--muted); }
.chk { display: flex; align-items: center; gap: 8px; font-size: 11.5px; }
.box { width: 13px; height: 13px; border-radius: 2px; border: 1px solid #6e7681; background: var(--sunken); display: inline-block; position: relative; flex: none; }
.box.on, .box.some { background: var(--accent); border-color: var(--accent); }
.box.on::after { content: ''; position: absolute; left: 3.5px; top: 0.5px; width: 3.5px; height: 7.5px; border: solid #fff; border-width: 0 2px 2px 0; transform: rotate(45deg); }
.box.some::after { content: ''; position: absolute; left: 2px; right: 2px; top: 5px; height: 2px; background: #fff; }
.help { font-size: 10.5px; color: var(--faint); line-height: 1.7; margin: 0; }
.actions { display: grid; grid-template-columns: 1fr 70px; gap: 8px; }
.btn { height: 30px; border-radius: 3px; border: 1px solid var(--line); background: transparent; color: var(--muted); font: 500 11.5px var(--sans); cursor: pointer; }
.btn.go { background: var(--go); border-color: var(--go-line); color: #fff; font-weight: 600; }
.btn.sm { height: 26px; padding: 0 10px; color: var(--text); }
.btn:disabled { opacity: .35; cursor: default; }
.tabs { display: flex; gap: 2px; padding: 0 10px; border-bottom: 1px solid var(--line); overflow-x: auto; }
.tabs button { background: none; border: 0; border-bottom: 2px solid transparent; color: var(--faint); padding: 14px 10px 12px; font-size: 12px; cursor: pointer; white-space: nowrap; display: inline-flex; align-items: center; gap: 6px; }
.tabs button[aria-selected="true"] { color: var(--text); border-bottom-color: #f78166; font-weight: 600; }
.tabs button em { font: normal 10px var(--mono); color: var(--faint); background: var(--sunken); border: 1px solid var(--line); border-radius: 8px; padding: 0 6px; }
.tabs button em.bad { color: var(--bad); border-color: rgba(248,81,73,.4); }
.tabs button:focus-visible { outline: 2px solid var(--accent); outline-offset: -2px; }
.tabs .spacer { margin-left: auto; align-self: center; font-size: 10.5px; color: var(--faint); white-space: nowrap; padding-left: 12px; }
.panel[hidden] { display: none !important; }
.panel .lead2 { padding: 14px 16px 0; font-size: 10.5px; color: var(--faint); }
.tools { display: flex; align-items: center; gap: 10px; padding: 14px 16px 0; flex-wrap: wrap; }
.search { margin-left: auto; width: 230px; max-width: 100%; height: 30px; border: 1px solid var(--line); background: var(--sunken); border-radius: 3px; padding: 0 10px; color: var(--text); font: 11px var(--sans); }
.labels { display: flex; flex-wrap: wrap; gap: 6px; padding: 12px 16px 14px; }
.lbl { display: inline-flex; align-items: center; gap: 7px; height: 25px; padding: 0 8px; border: 1px solid var(--line); border-radius: 3px; font-size: 11px; color: var(--muted); cursor: pointer; background: transparent; }
.lbl.on { border-color: var(--accent-line); color: var(--text); background: var(--accent-soft); }
.lbl em { font: normal 9.5px var(--mono); color: var(--faint); }
.lbl i { width: 7px; height: 7px; border-radius: 2px; display: inline-block; }
.tbl { overflow-x: auto; }
table { width: 100%; border-collapse: collapse; }
th { text-align: left; font-size: 10px; font-weight: 500; color: var(--faint); padding: 9px 16px; border-top: 1px solid var(--line); border-bottom: 1px solid var(--line); white-space: nowrap; }
td { padding: 11px 16px; border-bottom: 1px solid var(--line-soft); font-size: 11.5px; vertical-align: top; white-space: nowrap; }
tbody tr:last-child td { border-bottom: 0; }
.file { font: 600 11.5px var(--mono); }
.file i { font-style: normal; font-weight: 400; color: var(--faint); margin-right: 6px; font-size: 10px; }
.desc { color: var(--faint); font-size: 10.5px; margin-top: 3px; white-space: normal; }
.tag { display: inline-block; font: 9.5px var(--mono); color: var(--muted); border: 1px solid var(--line); border-radius: 2px; padding: 0 5px; margin-right: 3px; }
.mode { display: inline-block; font: 600 9.5px var(--mono); padding: 2px 6px; border-radius: 2px; background: var(--chip); color: var(--muted); border: 1px solid var(--line); }
.mode.serial { background: transparent; color: var(--text); border-color: var(--faint); }
.phase { display: inline-block; font: 700 9px var(--mono); letter-spacing: .06em; padding: 3px 6px; border-radius: 2px; background: var(--chip); color: var(--muted); }
.seq { color: var(--faint); font: 10.5px var(--mono); margin-left: 6px; }
.num { text-align: right; font-family: var(--mono); }
.mono { font-family: var(--mono); }
.good { color: var(--good); } .bad { color: var(--bad); } .warn { color: var(--warn); } .run { color: var(--accent); } .faint { color: var(--faint); }
.foot { display: flex; justify-content: space-between; align-items: center; gap: 16px; padding: 12px 16px; border-top: 1px solid var(--line); font-size: 10.5px; color: var(--faint); flex-wrap: wrap; }
.recent { border-top: 1px solid var(--line); }
details summary { list-style: none; cursor: pointer; padding: 16px; font-size: 13px; font-weight: 700; display: flex; align-items: baseline; gap: 12px; }
details summary::-webkit-details-marker { display: none; }
details summary::before { content: '▸'; font-size: 11px; color: var(--muted); }
details[open] summary::before { content: '▾'; }
details summary span { font-size: 10.5px; color: var(--faint); font-weight: 400; }
details pre { margin: 0; padding: 14px 16px; border-top: 1px solid var(--line); font: 11px/1.7 var(--mono); color: var(--muted); overflow-x: auto; }
.sub-details { border-top: 1px solid var(--line); }
.err { display: grid; grid-template-columns: 72px 170px minmax(0, 1fr); gap: 14px; padding: 10px 16px; border-bottom: 1px solid var(--line-soft); font: 11px var(--mono); }
.err:last-child { border-bottom: 0; }
.err span:last-child { white-space: normal; }
.wide { margin-top: 16px; display: grid; gap: 16px; }
.footer { max-width: 1000px; margin: 30px auto 0; padding: 18px max(16px, 24px) 26px; display: flex; justify-content: space-between; gap: 12px; font: 9.5px var(--mono); letter-spacing: .12em; color: var(--faint); flex-wrap: wrap; }
.fleet { padding: 14px 16px 4px; display: grid; gap: 10px; }
.fleet-top { display: flex; justify-content: space-between; align-items: baseline; gap: 10px; font-size: 10.5px; color: var(--faint); flex-wrap: wrap; }
.cells { display: grid; grid-template-columns: repeat(auto-fill, minmax(11px, 1fr)); gap: 3px; }
.cells i { aspect-ratio: 1; border-radius: 2px; background: var(--idle); display: block; cursor: pointer; }
.cells i.run { background: var(--accent); }
.cells i.slow { background: var(--warn); }
.cells i.lost { background: var(--bad); }
.cells i.sel { outline: 2px solid var(--text); outline-offset: 1px; }
.scale { display: flex; align-items: center; gap: 6px; font: 10px var(--mono); }
.scale b { display: inline-block; width: 60px; height: 7px; border-radius: 2px; background: linear-gradient(90deg, rgba(56,139,253,.15), var(--accent)); }
.pager { display: flex; align-items: center; gap: 8px; font: 10.5px var(--mono); }
.bar-cell { display: inline-block; width: 60px; height: 6px; background: var(--sunken); border: 1px solid var(--line); border-radius: 2px; vertical-align: middle; margin-right: 6px; }
.bar-cell i { display: block; height: 100%; background: var(--accent); }
.dist { padding: 0 16px 14px; }
.dist-bars { display: flex; align-items: flex-end; gap: 3px; height: 56px; }
.dist-bars i { flex: 1; background: var(--accent-soft); border-top: 2px solid var(--accent); }
.dist-axis { display: flex; justify-content: space-between; font: 9.5px var(--mono); color: var(--faint); margin-top: 4px; }
.chart-grid { display: grid; grid-template-columns: 1fr 1fr; }
.chart-grid > div { padding: 12px 14px 4px; border-bottom: 1px solid var(--line); min-width: 0; }
.chart-grid > div:nth-child(odd) { border-right: 1px solid var(--line); }
.chart-grid > div:nth-last-child(-n+2) { border-bottom: 0; }
.chart-grid h3 { margin: 0; font-size: 11px; font-weight: 500; color: var(--muted); display: flex; justify-content: space-between; }
.chart-grid h3 b { font: 600 11px var(--mono); color: var(--text); }
.chart { height: 170px; }
.mix { display: flex; height: 26px; border-radius: 3px; overflow: hidden; font: 10px/26px var(--mono); border: 1px solid var(--line); }
.mix span { padding: 0 6px; white-space: nowrap; overflow: hidden; }
.view[hidden] { display: none !important; }
@media (max-width: 860px) {
  .grid { grid-template-columns: minmax(0, 1fr); }
  .stats { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .chart-grid { grid-template-columns: minmax(0, 1fr); }
  .chart-grid > div { border-right: 0 !important; border-bottom: 1px solid var(--line) !important; }
}
@media (max-width: 480px) {
  .stats { grid-template-columns: minmax(0, 1fr); }
  .bar { gap: 14px; }
  .err { grid-template-columns: 60px minmax(0, 1fr); }
  .err span:nth-child(2) { display: none; }
}
"""

SUITES = [
    ('movement_test', '이동·충돌·맵 경계 검증', ['regular'], 2, False, True),
    ('attack_test', '평타·피해·처치 보상', ['regular'], 2, False, True),
    ('skill_test', '마법 시전·버프·쿨타임', ['regular'], 4, False, True),
    ('trade_test', '교환 요청·잠금·성사', ['regular'], 2, False, False),
    ('drop_loot_test', '드롭·줍기·죽은 자의 온기', ['regular'], 2, False, True),
    ('group_test', '그룹 결성·경험치 분배·드롭 차례', ['regular'], 6, False, True),
    ('marriage_test', '청혼·결혼·이혼', ['regular'], 2, True, False),
    ('marketplace_test', '마켓 등록·구매·정산', ['regular'], 3, True, False),
    ('rabbit_quest_test', '토끼의간 퀘스트 진행', ['regular', 'script'], 1, False, True),
    ('quest_lighthouse_test', '등대 퀘스트 보상', ['script'], 1, False, False),
    ('npc_explorer_test', 'NPC 대화 전체 탐색', ['explorer'], 16, False, False),
]

LABELS = [('regular', 42, 'some'), ('script', 9, ''), ('explorer', 1, ''), ('serial', 4, '')]
STATE_WORD = {'run': '실행 중', 'idle': '대기', 'slow': '보고 지연', 'lost': '끊김'}


def agents(seed, count):
    rng = random.Random(seed)
    hosts = ['DESKTOP-FB', 'BOT-NODE-{:02d}', '192.168.0.{}', 'ci-runner-{}']
    suites = [s[0] for s in SUITES[:9]]
    out = []
    for i in range(count):
        roll = rng.random()
        state = 'lost' if roll < 0.02 else 'slow' if roll < 0.06 else 'idle' if roll < 0.16 else 'run'
        host = rng.choice(hosts)
        host = host.format(rng.randint(1, 60)) if '{' in host else host
        seats = 4
        used = 0 if state in ('idle', 'lost') else rng.randint(1, seats)
        bots = 0 if state == 'lost' else rng.randint(90, 110)
        picks = rng.sample(suites, used)
        work = '—' if not picks else picks[0] if len(picks) == 1 else f'{picks[0]} 외 {len(picks) - 1}'
        out.append(dict(name=f'agent-{i + 1:03d}', host=host, state=state, seats=seats, used=used,
                        work=work, bots=bots,
                        cpu=0 if state == 'lost' else rng.randint(8, 72), seen=round(rng.uniform(0.1, 0.9), 1) if state in ('run', 'idle') else round(rng.uniform(4, 9), 1) if state == 'slow' else round(rng.uniform(10, 60), 1),
                        process=i + 1))
    return out


def stat(k, v, s):
    return f'<div class="card stat"><div class="k">{k}</div><div class="v">{v}</div><div class="s">{s}</div></div>'


def fld(label, value, sel=False):
    return f'<div class="fld"><label>{label}</label><div class="in{" sel" if sel else ""}">{value}</div></div>'


def header(active):
    links = ''.join(f'<a class="{"on" if key == active else ""}" data-view="{key}">{name}</a>' for key, name in (('integration', '통합 테스트'), ('load', '부하 테스트')))
    return f'<div class="bar"><div class="logo"><b>FB</b> / BOT CLUSTER</div><div class="views">{links}</div><div class="online">Master 온라인</div></div>'


def fleet_stat(fleet, sub):
    counts = {k: sum(1 for a in fleet if a['state'] == k) for k in STATE_WORD}
    colors = {'run': 'var(--accent)', 'idle': 'var(--idle)', 'slow': 'var(--warn)', 'lost': 'var(--bad)'}
    states = ''.join(f'<span><i style="background:{colors[k]}"></i>{STATE_WORD[k]} {counts[k]}</span>' for k in STATE_WORD)
    return f'<div class="card stat"><div class="k">연결된 Agent</div><div class="v">{len(fleet)}</div><div class="s states">{states}</div></div>'


def fleet_panel(prefix, mode):
    filters = ''.join(f'<button class="lbl{" on" if key == "all" else ""}" data-filter="{key}">{"<i style=background:" + color + "></i>" if color else ""}{word}<em data-count="{key}"></em></button>'
                      for key, word, color in [('all', '전체', ''), ('run', '실행 중', 'var(--accent)'), ('idle', '대기', 'var(--idle)'), ('slow', '보고 지연', 'var(--warn)'), ('lost', '끊김', 'var(--bad)')])
    legend = ('<span class="scale">봇 0<b></b>110</span>' if mode == 'load' else
              '<span>칸 하나가 Agent 한 대 · 누르면 아래 목록에서 찾음</span>')
    heads = ('<th>Agent</th><th>호스트</th><th>상태</th><th class="num">봇</th><th>CPU</th><th>계정 범위</th><th class="num">마지막 보고</th>' if mode == 'load' else
             '<th>Agent</th><th>호스트</th><th>상태</th><th>자리</th><th>실행 중 스위트</th><th>CPU</th><th class="num">마지막 보고</th>')
    return f"""<div class="fleet"><div class="fleet-top"><span>{"Agent별 봇 수" if mode == "load" else "Agent 상태"}</span>{legend}</div><div class="cells" id="{prefix}-cells"></div></div>
<div class="labels" id="{prefix}-filters">{filters}<input class="search" id="{prefix}-search" placeholder="Agent 이름 또는 호스트 검색" aria-label="Agent 검색"></div>
<div class="tbl"><table><thead><tr>{heads}</tr></thead><tbody id="{prefix}-rows"></tbody></table></div>
<div class="foot"><span id="{prefix}-range"></span><span class="pager"><button class="btn sm" id="{prefix}-prev">이전</button><button class="btn sm" id="{prefix}-next">다음</button></span></div>"""


def tabs(prefix, items, note=''):
    buttons = ''.join(f'<button role="tab" data-tab="{key}" aria-selected="{"true" if i == 0 else "false"}">{name}{"<em" + (" class=bad" if bad else "") + ">" + count + "</em>" if count else ""}</button>'
                      for i, (key, name, count, bad) in enumerate(items))
    return f'<div class="tabs" role="tablist" data-tabs="{prefix}">{buttons}{f"<span class=spacer>{note}</span>" if note else ""}</div>'


def integration(fleet):
    picks = ''.join(
        f'<tr><td><span class="box{" on" if on else ""}"></span></td><td><div class="file"><i>{i + 1:02d}</i>{name}</div><div class="desc">{desc}</div></td>'
        f'<td>{"".join(f"<span class=tag>{l}</span>" for l in labels)}</td><td class="num">{bots}</td>'
        f'<td><span class="mode{" serial" if serial else ""}">{"serial" if serial else "parallel"}</span></td></tr>'
        for i, (name, desc, labels, bots, serial, on) in enumerate(SUITES))
    labels = ''.join(f'<span class="lbl{" on" if mark else ""}"><span class="box {mark}"></span>{name}<em>{count}</em></span>' for name, count, mark in LABELS)
    plan = [
        (1, 'movement_test', '봇 2 · 1회 · 30초', '<span class="good">통과</span>', 'agent-014', '1 / 0 / 0', '0 / 0'),
        (2, 'attack_test', '봇 2 · 1회 · 30초', '<span class="bad">실패</span>', 'agent-052', '0 / 1 / 0', '0 / 0'),
        (3, 'skill_test', '봇 4 · 1회 · 60초', '<span class="run">실행 중</span>', 'agent-003', '0 / 0 / 0', '4 / 4'),
        (5, 'drop_loot_test', '봇 2 · 1회 · 30초', '<span class="good">통과</span>', 'agent-071', '1 / 0 / 0', '0 / 0'),
        (6, 'group_test', '봇 6 · 1회 · 60초', '<span class="run">실행 중</span>', 'agent-071', '0 / 0 / 0', '6 / 6'),
        (9, 'rabbit_quest_test', '봇 1 · 1회 · 30초', '<span class="warn">재배정</span>', 'agent-088 → agent-019', '0 / 0 / 1', '1 / 1'),
    ]
    plan_rows = ''.join(
        f'<tr><td><span class="phase">PARALLEL</span><span class="seq">#{seq:02d}</span></td>'
        f'<td><div class="file">{name}</div><div class="desc mono">{sub}</div></td><td>{state}</td><td class="mono">{agent}</td><td class="mono">{res}</td><td class="mono">{bots}</td></tr>'
        for seq, name, sub, state, agent, res, bots in plan)
    cases = [('movement/walk', 2, 0, 0, 412.4, 530.1), ('movement/warp', 2, 0, 0, 980.2, 1102.7), ('attack/hit', 1, 1, 0, 1505.0, 1810.3),
             ('drop_loot/pick', 2, 0, 0, 640.8, 702.9), ('skill/cast', 3, 0, 0, 821.5, 990.0)]
    case_rows = ''.join(f'<tr><td class="mono">{n}</td><td class="num">{a}</td><td class="num{" bad" if b else ""}">{b}</td><td class="num">{c}</td><td class="num">{d}</td><td class="num">{e}</td></tr>' for n, a, b, c, d, e in cases)
    used = sum(a['used'] for a in fleet)
    total = sum(a['seats'] for a in fleet if a['state'] != 'lost')
    tab_bar = tabs('itab', [('pick', '스위트 선택', '6 / 52', False), ('plan', '실행 계획', '3 / 6', False), ('agents', 'Agent', str(len(fleet)), False),
                            ('results', '결과', '', False), ('errors', '오류', '2', True)])
    return f"""<section class="view" id="integration">{header('integration')}<div class="page">
<div class="eyebrow">FB / registry.lua</div>
<div class="title"><div><h1>통합 테스트 실행 계획</h1><p class="lead">스위트를 이름과 label로 고르면, 병렬 스위트를 Agent의 빈 자리에 나눠 실행한 뒤 serial 스위트를 하나씩 실행합니다.</p></div><span class="status">RUNNING · 03:21</span></div>
<div class="stats">
{fleet_stat(fleet, '')}{stat('사용 중 자리 / 전체 자리', f'{used}<span>/</span>{total}', '스위트 하나가 자리 하나를 씀')}{stat('실행 중 스위트 / 선택', '2<span>/</span>6', '완료 3 · 대기 1 · serial 0')}
{stat('통과 스위트 / 완료 스위트', '2<span>/</span>3', '실패 1 · 재배정 1')}{stat('시나리오 성공 / 실패', '11<span>/</span>1', '취소 1 · 시나리오별 결과의 합계')}{stat('활성 봇 / 목표 봇', '10<span>/</span>17', 'gateway 127.0.0.1:3001 · 클라이언트 5.50')}
</div>
<div class="grid">
<div class="card"><div class="head"><h2>실행 설정</h2><span class="note">선택한 모든 스위트에 적용</span></div><div class="form">
<div class="two">{fld('스위트당 반복', '1')}{fld('병렬 스위트 상한', '전체 자리')}</div>
<div class="two">{fld('스위트 제한 시간 (초)', '120')}{fld('Agent 끊김 재시도', '1')}</div>
<div class="chk"><span class="box on"></span>실패한 스위트 로그 꼬리 저장</div>
<p class="help">serial 스위트는 서버 전역 상태(결혼·마켓·보관함)를 쓰므로 병렬 단계가 끝난 뒤 클러스터 전체에서 하나씩 실행합니다.</p>
{fld('서버 환경', 'localhost', True)}
<div class="two">{fld('gateway 호스트', '127.0.0.1')}{fld('포트', '3001')}</div>
{fld('클라이언트 버전', '5.50', True)}
{fld('사용할 Agent', '전체 (실행 중 · 대기)', True)}
<p class="help">통합 테스트는 실행마다 새 계정을 만들어 Agent끼리 계정이 겹치지 않습니다.</p>
<div class="actions"><button class="btn go">선택한 6개 스위트 실행</button><button class="btn">중지</button></div>
</div></div>
<div class="card recent">{tab_bar}
<div class="panel" data-panel="itab:pick">
<div class="tools"><span class="chk"><span class="box some"></span>전체 선택</span><button class="btn sm">선택 해제</button><button class="btn sm">실패만 다시 선택</button><input class="search" placeholder="스위트 이름 또는 설명 검색" aria-label="스위트 검색"></div>
<div class="labels">{labels}</div>
<div class="tbl"><table><thead><tr><th>선택</th><th>순서 / 스위트</th><th>label</th><th class="num">봇</th><th>실행 방식</th></tr></thead><tbody>{picks}</tbody></table></div>
<div class="foot"><span>label을 누르면 그 label의 스위트를 모두 고릅니다 · Shift+클릭은 빼기</span><span>registry.lua 등록 순서</span></div></div>
<div class="panel" data-panel="itab:plan" hidden>
<div class="tbl"><table><thead><tr><th>단계 / 순서</th><th>스위트</th><th>상태</th><th>Agent</th><th>성공 / 실패 / 취소</th><th>봇 / 연결</th></tr></thead><tbody>{plan_rows}</tbody></table></div>
<div class="foot"><span>병렬 스위트는 빈 자리가 생기는 순서로 시작하므로 끝나는 순서는 다를 수 있습니다. serial 스위트는 하나가 끝나야 다음을 시작합니다.</span></div></div>
<div class="panel" data-panel="itab:agents" hidden>{fleet_panel('ia', 'integration')}</div>
<div class="panel" data-panel="itab:results" hidden>
<div class="tbl"><table><thead><tr><th>시나리오</th><th class="num">성공</th><th class="num">실패</th><th class="num">취소</th><th class="num">평균 ms</th><th class="num">최대 ms</th></tr></thead><tbody>{case_rows}</tbody></table></div>
<details class="sub-details"><summary>요청·이벤트·상태 대기 통계<span>전체 스위트 합계 · ms</span></summary></details></div>
<div class="panel" data-panel="itab:errors" hidden>
<div class="err"><span class="faint">03:20:15</span><span>attack_test · agent-052</span><span class="bad">scenario 2: 대상 HP가 줄지 않음 (기대 &lt; 100, 실제 100)</span></div>
<div class="err"><span class="faint">02:58:40</span><span>rabbit_quest_test</span><span class="warn">agent-088 하트비트 10초 끊김 → agent-019에 재배정</span></div></div>
</div></div>
<div class="wide">
<details class="card"><summary>스위트 스크립트<span>attack_test.lua</span></summary><pre>test_suite {{
    name      = "Attack Test",
    bot_count = 2,
    scenarios = {{ ... }},
}}</pre></details>
<details class="card"><summary>등록 순서와 label·serial 정의<span>scripts/integration/registry.lua</span></summary></details>
</div></div></section>"""


def series(seed, length, base, noise, ramp=0):
    rng = random.Random(seed)
    raw = [rng.uniform(-noise, noise) for _ in range(length)]
    out = []
    for i in range(length):
        window = raw[max(0, i - 4):i + 1]
        grow = min(1.0, i / ramp) if ramp else 1.0
        out.append(round(max(0, base * grow + sum(window) / len(window) + noise * 0.4 * math.sin(i / 9 + seed)), 1))
    return out


def load(fleet):
    rows = [('brain:fight', 'brain', 89450, 140, 620, 2100, 3400, 118.3), ('game:item_update', 'packet', 148220, 12, 14, 48, 88, 196.1),
            ('brain:warp', 'brain', 34110, 88, 110, 320, 510, 45.1), ('game:spell_cast', 'packet', 61230, 31, 22, 70, 130, 81.0),
            ('brain:buy', 'brain', 5120, 3, 80, 210, 300, 6.8), ('brain:quest', 'brain', 410, 5, 2800, 9100, 12800, 0.5)]
    body = ''.join(f'<tr><td class="mono">{n}</td><td><span class="tag">{k}</span></td><td class="num">{c:,}</td><td class="num{" bad" if f else ""}">{f}</td>'
                   f'<td class="num">{a:,}</td><td class="num">{b:,}</td><td class="num">{d:,}</td><td class="num">{r}</td></tr>' for n, k, c, f, a, b, d, r in rows)
    live = [a for a in fleet if a['state'] != 'lost']
    bots = sum(a['bots'] for a in live)
    buckets = [0] * 11
    for a in live:
        buckets[min(10, max(0, a['bots'] - 90) // 2)] += 1
    peak = max(buckets) or 1
    dist = ''.join(f'<i style="height:{max(4, round(v / peak * 100))}%" title="{90 + i * 2}~{91 + i * 2}봇 · {v}대"></i>' for i, v in enumerate(buckets))
    tab_bar = tabs('ltab', [('requests', '요청별 통계', str(len(rows)), False), ('agents', 'Agent', str(len(fleet)), False), ('errors', '오류', '3', True)])
    return f"""<section class="view" id="load" hidden>{header('load')}<div class="page">
<div class="eyebrow">FB / config/load.json</div>
<div class="title"><div><h1>부하 테스트</h1><p class="lead">봇 수와 증가 속도를 Agent 용량 비율로 나눕니다. Agent가 들어오거나 나가면 남은 Agent끼리 다시 나눕니다.</p></div><span class="status">RUNNING · 12:34 / 30:00</span></div>
<div class="stats">
{fleet_stat(fleet, '')}{stat('접속 봇 / 목표 봇', f'{bots:,}<span>/</span>{len(live) * 100:,}', f'Agent당 평균 {bots // len(live)} · 최소 90 · 최대 110')}{stat('초당 요청', '12,410', '최근 5분 최고 13,100')}
{stat('실패율', '0.8%', '초당 실패 99')}{stat('응답 시간 p95', '182 ms', 'p50 42 · p99 310')}{stat('분당 처치 / 사망', '4,810<span>/</span>21', '분당 경험치 890k')}
</div>
<div class="grid">
<div class="card"><div class="head"><h2>실행 설정</h2><span class="note">실행 중 변경 가능</span></div><div class="form">
<div class="two">{fld('봇 수', f'{len(live) * 100:,}')}{fld('초당 증가', '50')}</div>
<div class="two">{fld('Agent당 상한', '110')}{fld('지속 시간 (분)', '30')}</div>
<div class="fld"><label>persona 비율</label><div class="mix"><span style="flex:60;background:rgba(110,118,129,.30)">hunter 60</span><span style="flex:20;background:rgba(110,118,129,.18)">trader 20</span><span style="flex:20;background:rgba(110,118,129,.08)">social 20</span></div></div>
{fld('시나리오', '기본 (setup 없음)', True)}{fld('클라이언트 버전', '5.50', True)}{fld('계정 세트', '#6000 · 이어서 진행', True)}
<p class="help">봇 수를 Agent 용량 비율로 나눕니다. 계정 번호는 Master가 Agent마다 process × 1,000,000 범위로 발급하고, 세트 번호는 그 안의 오프셋입니다. 같은 세트는 지난 실행의 캐릭터(레벨·장비)를 이어 쓰고, 새 세트는 1레벨부터 시작합니다.</p>
<div class="actions"><button class="btn go">봇 수 적용</button><button class="btn">중지</button></div>
</div></div>
<div class="stack">
<div class="card recent"><div class="head"><h2>현황</h2><span class="note">1초 간격 · 최근 12분 · 전체 Agent 합계</span></div>
<div class="chart-grid">
<div><h3>초당 요청 · 실패<b>12,410</b></h3><div class="chart" id="c1"></div></div>
<div><h3>응답 시간 p50 · p95 · p99<b>182 ms</b></h3><div class="chart" id="c2"></div></div>
<div><h3>접속 봇 · 연결된 Agent<b>{bots:,}</b></h3><div class="chart" id="c3"></div></div>
<div><h3>분당 처치 · 사망<b>4,810</b></h3><div class="chart" id="c4"></div></div>
</div></div>
<div class="card">{tab_bar}
<div class="panel" data-panel="ltab:requests">
<div class="tbl"><table><thead><tr><th>이름</th><th>종류</th><th class="num">요청</th><th class="num">실패</th><th class="num">p50</th><th class="num">p95</th><th class="num">p99</th><th class="num">초당</th></tr></thead><tbody>{body}</tbody></table></div>
<div class="foot"><span>누적 · ms · 모든 Agent의 응답 시간 분포를 합친 값</span></div></div>
<div class="panel" data-panel="ltab:agents" hidden>
<div class="lead2">Agent별 봇 수 분포</div><div class="dist"><div class="dist-bars">{dist}</div><div class="dist-axis"><span>90봇</span><span>100봇</span><span>110봇</span></div></div>
{fleet_panel('la', 'load')}</div>
<div class="panel" data-panel="ltab:errors" hidden>
<div class="err"><span class="faint">12:33:48</span><span>brain:warp · agent-052</span><span class="bad">TIMEOUT · 320 → 321 워프 응답 없음</span></div>
<div class="err"><span class="faint">12:33:12</span><span>brain:fight · agent-014</span><span class="bad">NO_HIT · 대상에 공격이 닿지 않음</span></div>
<div class="err"><span class="faint">12:32:05</span><span>brain:buy · agent-071</span><span class="warn">MONEY_SHORT · 노란비서 금전 부족</span></div></div>
</div>
</div></div></div></section>"""


SCRIPT = """
const D = __DATA__;
const WORD = { run: '실행 중', idle: '대기', slow: '보고 지연', lost: '끊김' };
const CLS = { run: 'good', idle: 'faint', slow: 'warn', lost: 'bad' };
const PAGE = 12;

function fleet(prefix, mode) {
  const cells = document.getElementById(prefix + '-cells');
  const rows = document.getElementById(prefix + '-rows');
  const range = document.getElementById(prefix + '-range');
  const search = document.getElementById(prefix + '-search');
  const filters = document.getElementById(prefix + '-filters');
  let filter = 'all', page = 0, picked = null;
  filters.querySelectorAll('[data-count]').forEach(e => e.textContent = e.dataset.count === 'all' ? D.agents.length : D.agents.filter(a => a.state === e.dataset.count).length);
  cells.innerHTML = D.agents.map((a, i) => {
    const style = mode === 'load' && a.state === 'run' ? ` style="opacity:${(0.25 + 0.75 * (a.bots - 90) / 20).toFixed(2)}"` : '';
    return `<i class="${a.state}" data-i="${i}" title="${a.name} · ${a.host} · ${WORD[a.state]}${mode === 'load' ? ' · 봇 ' + a.bots : ' · 자리 ' + a.used + '/' + a.seats}"${style}></i>`;
  }).join('');
  const list = () => D.agents.filter(a => (filter === 'all' || a.state === filter) && (a.name + a.host).toLowerCase().includes(search.value.trim().toLowerCase()));
  const render = () => {
    const all = list();
    page = Math.min(page, Math.max(0, Math.ceil(all.length / PAGE) - 1));
    const shown = all.slice(page * PAGE, page * PAGE + PAGE);
    rows.innerHTML = shown.map(a => {
      const cpu = `<span class="bar-cell"><i style="width:${a.cpu}%"></i></span>${a.cpu}%`;
      const seen = `${a.seen}초 전`;
      const mark = picked === a.name ? ' style="background:var(--accent-soft)"' : '';
      return mode === 'load'
        ? `<tr${mark}><td class="mono">${a.name}</td><td class="mono faint">${a.host}</td><td class="${CLS[a.state]}">${WORD[a.state]}</td><td class="num">${a.bots}</td><td class="mono">${cpu}</td><td class="mono faint">${(a.process * 1000000 + 6000).toLocaleString()}~</td><td class="num">${seen}</td></tr>`
        : `<tr${mark}><td class="mono">${a.name}</td><td class="mono faint">${a.host}</td><td class="${CLS[a.state]}">${WORD[a.state]}</td><td class="mono">${a.used} / ${a.seats}</td><td class="mono">${a.work}</td><td class="mono">${cpu}</td><td class="num">${seen}</td></tr>`;
    }).join('');
    range.textContent = all.length ? `${page * PAGE + 1}–${page * PAGE + shown.length} / ${all.length}대` : '조건에 맞는 Agent가 없습니다';
    document.getElementById(prefix + '-prev').disabled = page === 0;
    document.getElementById(prefix + '-next').disabled = (page + 1) * PAGE >= all.length;
    cells.querySelectorAll('i').forEach(c => c.classList.toggle('sel', D.agents[c.dataset.i].name === picked));
  };
  filters.addEventListener('click', e => {
    const b = e.target.closest('[data-filter]');
    if (!b) return;
    filter = b.dataset.filter; page = 0;
    filters.querySelectorAll('[data-filter]').forEach(x => x.classList.toggle('on', x === b));
    render();
  });
  search.addEventListener('input', () => { page = 0; render(); });
  cells.addEventListener('click', e => {
    const c = e.target.closest('i');
    if (!c) return;
    const a = D.agents[c.dataset.i];
    picked = a.name; filter = 'all'; search.value = '';
    filters.querySelectorAll('[data-filter]').forEach(x => x.classList.toggle('on', x.dataset.filter === 'all'));
    page = Math.floor(D.agents.indexOf(a) / PAGE);
    render();
  });
  document.getElementById(prefix + '-prev').addEventListener('click', () => { page--; render(); });
  document.getElementById(prefix + '-next').addEventListener('click', () => { page++; render(); });
  render();
}

document.querySelectorAll('[data-tabs]').forEach(bar => {
  const group = bar.dataset.tabs;
  bar.addEventListener('click', e => {
    const b = e.target.closest('[data-tab]');
    if (!b) return;
    bar.querySelectorAll('[data-tab]').forEach(x => x.setAttribute('aria-selected', String(x === b)));
    document.querySelectorAll(`[data-panel^="${group}:"]`).forEach(p => p.hidden = p.dataset.panel !== `${group}:${b.dataset.tab}`);
  });
});

let drawn = false;
function draw() {
  if (drawn || !window.echarts) return;
  drawn = true;
  const base = { animation: false, grid: { left: 44, right: 36, top: 22, bottom: 20 },
    tooltip: { trigger: 'axis', backgroundColor: '#161b22', borderColor: '#30363d', textStyle: { color: '#e6edf3', fontSize: 11 } },
    legend: { top: 0, right: 0, icon: 'rect', itemWidth: 10, itemHeight: 2, textStyle: { color: '#8d96a0', fontSize: 10 } },
    xAxis: { type: 'category', data: D.x, boundaryGap: false, axisLine: { lineStyle: { color: '#30363d' } }, axisTick: { show: false }, axisLabel: { color: '#6e7681', fontSize: 9.5 } },
    yAxis: { type: 'value', splitLine: { lineStyle: { color: '#21262d' } }, axisLabel: { color: '#6e7681', fontSize: 9.5 } } };
  const line = (name, data, color, extra) => Object.assign({ name, type: 'line', data, showSymbol: false, smooth: 0.3, lineStyle: { width: 1.4, color }, itemStyle: { color } }, extra || {});
  const put = (id, series, extra) => echarts.init(document.getElementById(id)).setOption(Object.assign({}, base, { series }, extra || {}));
  put('c1', [line('초당 요청', D.rps, '#4493f8', { areaStyle: { color: 'rgba(56,139,253,0.08)' } }), line('초당 실패', D.fails, '#f85149')]);
  put('c2', [line('p50', D.p50, '#6e7681'), line('p95', D.p95, '#4493f8'), line('p99', D.p99, '#8d96a0', { lineStyle: { width: 1, type: 'dashed', color: '#8d96a0' } })]);
  put('c3', [line('접속 봇', D.users, '#4493f8', { areaStyle: { color: 'rgba(56,139,253,0.08)' } }), line('Agent', D.agentCount, '#8d96a0', { yAxisIndex: 1, step: 'end', smooth: false })],
      { yAxis: [base.yAxis, Object.assign({}, base.yAxis, { splitLine: { show: false } })] });
  put('c4', [line('처치', D.kills, '#4493f8'), line('사망', D.deaths, '#8d96a0')]);
}
function show(id) {
  document.querySelectorAll('.view').forEach(v => v.hidden = v.id !== id);
  if (id === 'load') draw();
  scrollTo(0, 0);
}
document.querySelectorAll('.views a').forEach(a => a.addEventListener('click', () => show(a.dataset.view)));
fleet('ia', 'integration');
fleet('la', 'load');
show(location.hash === '#load' ? 'load' : 'integration');
"""


def main():
    n = 152
    fleet = agents(7, 96)
    agent_count = [min(96, 40 + (i // 12) * 8) for i in range(n)]
    users = [round(min(9300, c * 100 * min(1.0, i / 60))) for i, c in enumerate(agent_count)]
    data = dict(x=[f'{m // 60:02d}:{m % 60:02d}' for m in range(0, n * 5, 5)], rps=series(1, n, 12410, 500, 60), fails=series(2, n, 99, 25, 60),
                p50=series(3, n, 42, 4), p95=series(4, n, 182, 18, 20), p99=series(5, n, 310, 30, 20), users=users, agentCount=agent_count,
                kills=series(6, n, 4810, 300, 60), deaths=series(8, n, 21, 6, 60), agents=fleet)
    footer = '<div class="footer"><span>FB BOT CLUSTER</span><span>REGISTER → SELECT → PARALLEL → SERIAL → RESULTS</span></div>'
    html = (f'<!doctype html><html lang="ko"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">'
            f'<title>봇 클러스터</title><link rel="stylesheet" href="{FONTS}"><style>{STYLE}</style></head><body>'
            f'{integration(fleet)}{load(fleet)}{footer}<script src="{ECHARTS}"></script>'
            f'<script>{SCRIPT.replace("__DATA__", json.dumps(data, ensure_ascii=False))}</script></body></html>')
    OUTPUT.mkdir(parents=True, exist_ok=True)
    target = OUTPUT / 'cluster.html'
    target.write_text(html, encoding='utf-8')
    print(target)


if __name__ == '__main__':
    main()
