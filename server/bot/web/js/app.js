'use strict';

const WORD = { run: '실행 중', idle: '대기', slow: '보고 지연', lost: '끊김' };
const CLS = { run: 'good', idle: 'faint', slow: 'warn', lost: 'bad' };
const COLOR = { run: 'var(--accent)', idle: 'var(--idle)', slow: 'var(--warn)', lost: 'var(--bad)' };
const SUITE_WORD = { pending: '대기', assigned: '배정', running: '실행 중', passed: '통과', failed: '실패', lost: '끊김', stopped: '중지' };
const SUITE_CLS = { pending: 'faint', assigned: 'run', running: 'run', passed: 'good', failed: 'bad', lost: 'warn', stopped: 'faint' };
const PAGE = 12;

const $ = id => document.getElementById(id);
const esc = value => String(value ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const num = value => Number(value || 0).toLocaleString('ko-KR');
const clock = seconds => {
  const total = Math.max(0, Math.round(seconds || 0));
  const h = Math.floor(total / 3600), m = Math.floor(total / 60) % 60, s = total % 60;
  const pad = v => String(v).padStart(2, '0');
  return h ? `${h}:${pad(m)}:${pad(s)}` : `${pad(m)}:${pad(s)}`;
};
const local = iso => iso ? new Date(iso).toLocaleString('ko-KR', { hour12: false }) : '-';
const kind = name => name.startsWith('goal:') ? 'goal' : name.startsWith('action:') ? 'action' : 'packet';

const store = {
  get(key, fallback) { try { const v = localStorage.getItem(key); return v === null ? fallback : JSON.parse(v); } catch { return fallback; } },
  set(key, value) { try { localStorage.setItem(key, JSON.stringify(value)); } catch { } },
};

const TWEEN_MS = 2000;
const tweens = new Map();

function frame(now) {
  tweens.forEach((t, el) => {
    const p = Math.min(1, (now - t.start) / TWEEN_MS);
    t.now = t.to.map((v, i) => t.from[i] + (v - t.from[i]) * p);
    const html = t.format(t.now);
    if (el.innerHTML !== html)
      el.innerHTML = html;
    if (p >= 1)
      t.done = true;
  });
  if ([...tweens.values()].some(t => !t.done))
    requestAnimationFrame(frame);
}

function count(id, values, format) {
  const el = $(id);
  const prev = tweens.get(el);
  if (!values.every(Number.isFinite) || !prev || prev.now.length !== values.length || !prev.now.every(Number.isFinite)) {
    tweens.set(el, { from: values, to: values, now: values, start: 0, format, done: true });
    el.innerHTML = values.every(Number.isFinite) ? format(values) : '-';
    return;
  }
  if (prev.to.every((v, i) => v === values[i]))
    return;
  const idle = [...tweens.values()].every(t => t.done);
  tweens.set(el, { from: prev.now, to: values, now: prev.now, start: performance.now(), format, done: false });
  if (idle)
    requestAnimationFrame(frame);
}

const pair = values => `${num(Math.round(values[0]))}<span>/</span>${num(Math.round(values[1]))}`;
const whole = values => num(Math.round(values[0]));

function sync(host, items, key, make, update) {
  [...host.children].filter(e => e.dataset.key === undefined).forEach(e => e.remove());
  const old = new Map([...host.children].map(e => [e.dataset.key, e]));
  items.forEach((item, i) => {
    const k = String(key(item, i));
    let el = old.get(k);
    const fresh = !el;
    if (fresh) {
      el = make(item);
      el.dataset.key = k;
    } else {
      old.delete(k);
    }
    if (host.children[i] !== el)
      host.insertBefore(el, host.children[i] || null);
    if (fresh)
      el.getBoundingClientRect();
    update(el, item, i);
  });
  old.forEach(e => e.remove());
}

const S = {
  state: null,
  clock: { id: null, base: 0, at: 0, live: false, shown: 0, duration: 0, kind: null },
  catalog: null,
  selected: new Set(store.get('fb.cluster.selected', [])),
  search: '',
  view: 'integration',
  loadRun: null,
  charts: {},
  fleets: {},
};

async function api(path, body) {
  const options = body === undefined ? {} : { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) };
  const res = await fetch(path, options);
  const json = await res.json().catch(() => ({}));
  if (!res.ok)
    throw new Error(json.error || res.statusText);
  return json;
}

function notice(id, text, bad) {
  const box = $(id);
  box.hidden = !text;
  box.textContent = text || '';
  box.classList.toggle('bad', !!bad);
}

function fleet(host, mode) {
  host.innerHTML = `<div class="fleet"><div class="fleet-top"><span>${mode === 'load' ? 'Agent별 봇 수' : 'Agent 상태'}</span>
<span>${mode === 'load' ? '<span class="scale">봇 0<b></b><span data-max>0</span></span>' : '칸 하나가 Agent 한 대 · 누르면 아래 목록에서 찾음'}</span></div><div class="cells"></div></div>
<div class="labels">${[['all', '전체'], ['run', '실행 중'], ['idle', '대기'], ['slow', '보고 지연'], ['lost', '끊김']].map(([key, word]) =>
    `<button class="lbl${key === 'all' ? ' on' : ''}" data-filter="${key}">${key === 'all' ? '' : `<i style="background:${COLOR[key]}"></i>`}${word}<em data-count="${key}"></em></button>`).join('')}
<input class="search" placeholder="Agent 이름 또는 호스트 검색" aria-label="Agent 검색"></div>
<div class="tbl"><table><thead><tr>${mode === 'load'
    ? '<th>Agent</th><th>호스트</th><th>상태</th><th class="num">봇 / 몫</th><th class="num">용량</th><th>계정 범위</th><th class="num">마지막 보고</th><th></th>'
    : '<th>Agent</th><th>호스트</th><th>상태</th><th class="num">자리</th><th>맡은 스위트</th><th>인스턴스 자리</th><th class="num">마지막 보고</th><th></th>'}</tr></thead><tbody></tbody></table></div>
<div class="foot"><span data-range></span><span class="pager"><button class="btn sm" data-prev>이전</button><button class="btn sm" data-next>다음</button></span></div>`;

  const cells = host.querySelector('.cells');
  const rows = host.querySelector('tbody');
  const search = host.querySelector('.search');
  const filters = host.querySelector('.labels');
  let filter = 'all', page = 0, picked = null, agents = [];

  const list = () => agents.filter(a => (filter === 'all' || a.state === filter) &&
    `${a.name} ${a.host}`.toLowerCase().includes(search.value.trim().toLowerCase()));

  const render = () => {
    const all = list();
    page = Math.min(page, Math.max(0, Math.ceil(all.length / PAGE) - 1));
    const shown = all.slice(page * PAGE, page * PAGE + PAGE);
    rows.innerHTML = shown.map(a => {
      const mark = picked === a.id ? ' style="background:var(--accent-soft)"' : '';
      const seen = a.state === 'lost' ? '-' : `${a.seen}초 전`;
      const quit = a.state === 'lost' ? '' : `<button class="btn sm" data-quit="${a.id}">종료</button>`;
      const work = a.batch.length ? (a.batch.length === 1 ? a.batch[0] : `${a.batch[0]} 외 ${a.batch.length - 1}`) : '—';
      return mode === 'load'
        ? `<tr${mark}><td class="mono">${esc(a.name)}</td><td class="mono faint">${esc(a.host)}</td><td class="${CLS[a.state]}">${WORD[a.state]}${a.excluded ? ' · 제외' : ''}</td><td class="num">${num(a.bots)} / ${num(a.users)}</td><td class="num">${num(a.max)}</td><td class="mono faint">${num(a.process * 1000000)}~</td><td class="num">${seen}</td><td>${quit}</td></tr>`
        : `<tr${mark}><td class="mono">${esc(a.name)}</td><td class="mono faint">${esc(a.host)}</td><td class="${CLS[a.state]}">${WORD[a.state]}</td><td class="num">${a.seats}</td><td class="mono" title="${esc(a.batch.join(', '))}">${esc(work)}</td><td class="mono faint">${a.seat_base + 1}~${a.seat_base + a.seats + 1}</td><td class="num">${seen}</td><td>${quit}</td></tr>`;
    }).join('');
    host.querySelector('[data-range]').textContent = all.length ? `${page * PAGE + 1}–${page * PAGE + shown.length} / ${all.length}대` : '조건에 맞는 Agent가 없습니다';
    host.querySelector('[data-prev]').disabled = page === 0;
    host.querySelector('[data-next]').disabled = (page + 1) * PAGE >= all.length;
    cells.querySelectorAll('i').forEach(c => c.classList.toggle('sel', Number(c.dataset.id) === picked));
  };

  filters.addEventListener('click', e => {
    const button = e.target.closest('[data-filter]');
    if (!button)
      return;
    filter = button.dataset.filter;
    page = 0;
    filters.querySelectorAll('[data-filter]').forEach(x => x.classList.toggle('on', x === button));
    render();
  });
  search.addEventListener('input', () => { page = 0; render(); });
  cells.addEventListener('click', e => {
    const cell = e.target.closest('i');
    if (!cell)
      return;
    picked = Number(cell.dataset.id);
    filter = 'all';
    search.value = '';
    filters.querySelectorAll('[data-filter]').forEach(x => x.classList.toggle('on', x.dataset.filter === 'all'));
    page = Math.floor(agents.findIndex(a => a.id === picked) / PAGE);
    render();
  });
  rows.addEventListener('click', async e => {
    const button = e.target.closest('[data-quit]');
    if (!button)
      return;
    button.disabled = true;
    try { await api(`/api/agents/${button.dataset.quit}/quit`, {}); } catch (error) { button.disabled = false; }
  });
  host.querySelector('[data-prev]').addEventListener('click', () => { page--; render(); });
  host.querySelector('[data-next]').addEventListener('click', () => { page++; render(); });

  return list => {
    agents = list;
    const peak = Math.max(1, ...agents.map(a => a.bots));
    filters.querySelectorAll('[data-count]').forEach(e => e.textContent = e.dataset.count === 'all' ? agents.length : agents.filter(a => a.state === e.dataset.count).length);
    const max = host.querySelector('[data-max]');
    if (max)
      max.textContent = num(peak);
    sync(cells, agents, a => a.id, a => {
      const cell = document.createElement('i');
      cell.dataset.id = a.id;
      return cell;
    }, (cell, a) => {
      cell.className = a.state;
      cell.style.opacity = mode === 'load' && a.state === 'run' ? (0.25 + 0.75 * a.bots / peak).toFixed(2) : '';
      cell.title = `${a.name} · ${a.host} · ${WORD[a.state]}${mode === 'load' ? ` · 봇 ${a.bots}` : ` · 자리 ${a.seats}`}`;
    });
    render();
  };
}

function fleetSummary(agents) {
  return ['run', 'idle', 'slow', 'lost'].map(k => `<span><i style="background:${COLOR[k]}"></i>${WORD[k]} ${agents.filter(a => a.state === k).length}</span>`).join('');
}

function statsRows(rows, columns) {
  if (!rows.length)
    return `<tr><td colspan="${columns}" class="empty">아직 기록이 없습니다</td></tr>`;
  return rows;
}

function renderCatalog() {
  const catalog = S.catalog;
  if (!catalog)
    return;
  const known = new Set(catalog.suites.map(s => s.name));
  S.selected = new Set([...S.selected].filter(n => known.has(n)));
  store.set('fb.cluster.selected', [...S.selected]);

  const labels = {};
  catalog.suites.forEach(s => s.labels.concat(s.serial ? ['serial'] : []).forEach(l => (labels[l] = labels[l] || []).push(s.name)));
  $('i-labels').innerHTML = Object.entries(labels).map(([label, names]) => {
    const count = names.filter(n => S.selected.has(n)).length;
    const mark = count === 0 ? '' : count === names.length ? 'on' : 'some';
    return `<button class="lbl${mark ? ' on' : ''}" data-label="${esc(label)}"><span class="box ${mark}"></span>${esc(label)}<em>${names.length}</em></button>`;
  }).join('');

  const query = S.search.trim().toLowerCase();
  const rows = catalog.suites.map((s, i) => [s, i]).filter(([s]) => !query || s.name.toLowerCase().includes(query));
  $('i-suites').innerHTML = rows.map(([s, i]) => `<tr class="pick" data-suite="${esc(s.name)}"><td><span class="box${S.selected.has(s.name) ? ' on' : ''}"></span></td>
<td><div class="file"><i>${String(i + 1).padStart(2, '0')}</i>${esc(s.name)}</div>${s.error ? `<div class="desc bad">${esc(s.error)}</div>` : ''}</td>
<td>${s.labels.map(l => `<span class="tag">${esc(l)}</span>`).join('')}</td><td class="num">${s.bots}</td>
<td><span class="mode${s.serial ? ' serial' : ''}">${s.serial ? 'serial' : 'parallel'}</span>${s.extra_slot ? ' <span class="tag">extra</span>' : ''}</td></tr>`).join('');

  $('i-tab-pick').textContent = `${S.selected.size} / ${catalog.suites.length}`;
  const loaded = `${local(catalog.loaded_at)} 읽음 · 스위트 ${catalog.suites.length}개${catalog.files.some(f => f.error) ? ' · 가중치 파일 오류' : ''}`;
  $('i-loaded').textContent = loaded;
  $('l-loaded').textContent = loaded;
  $('footer-rev').textContent = `catalog r${catalog.revision}`;
  updateIntegrationButtons();
}

function updateIntegrationButtons() {
  const run = S.state && S.state.run;
  const active = !!run && run.state !== 'done';
  $('i-start').disabled = active || S.selected.size === 0;
  $('i-start').textContent = `선택한 ${S.selected.size}개 스위트 실행`;
  $('i-stop').disabled = !(active && run.kind === 'integration' && run.state === 'running');
  $('i-reload').disabled = active;
  $('l-reload').disabled = active;
}

function renderIntegration() {
  const state = S.state;
  const agents = state.agents;
  const run = state.run && state.run.kind === 'integration' ? state.run : null;
  const live = agents.filter(a => a.state !== 'lost');

  count('i-agents', [live.length], whole);
  $('i-fleet').innerHTML = fleetSummary(agents);
  count('i-seats', [agents.filter(a => a.state === 'run').length, live.reduce((t, a) => t + a.seats, 0)], pair);
  $('i-tab-agents').textContent = agents.length;
  S.fleets.ia(agents);

  const suites = run ? run.suites : [];
  const done = suites.filter(s => ['passed', 'failed', 'lost', 'stopped'].includes(s.state));
  const passed = suites.filter(s => s.state === 'passed').length;
  const failed = suites.filter(s => s.state === 'failed' || s.state === 'lost');
  const running = suites.filter(s => s.state === 'running' || s.state === 'assigned').length;
  const pending = suites.filter(s => s.state === 'pending').length;
  count('i-progress', [done.length, suites.length], pair);
  $('i-meter').style.width = suites.length ? `${(done.length - failed.length) / suites.length * 100}%` : '0%';
  $('i-meter-bad').style.width = suites.length ? `${failed.length / suites.length * 100}%` : '0%';
  $('i-progress-sub').textContent = run ? `실행 중 ${running} · 대기 ${pending}` : '-';
  count('i-result', [passed, failed.length], pair);
  $('i-result-sub').textContent = run ? `중지 ${suites.filter(s => s.state === 'stopped').length} · 재시도 ${suites.reduce((t, s) => t + s.attempts, 0)}` : '-';
  const packets = run ? run.stats.filter(r => kind(r.name) === 'packet') : [];
  count('i-requests', [packets.reduce((t, r) => t + r.count, 0), packets.reduce((t, r) => t + r.failures, 0)], pair);
  $('i-started').textContent = run ? `${local(run.started_at)} 시작 · ${run.id}` : '-';

  $('i-tab-plan').textContent = run ? `${done.length} / ${suites.length}` : '';
  $('i-plan-note').textContent = run ? `${done.length} / ${suites.length}개 종료 · ${run.state === 'done' ? '끝남' : running ? '진행 중' : '대기 중'}` : '-';
  $('i-plan-meter').style.width = $('i-meter').style.width;
  $('i-plan-meter-bad').style.width = $('i-meter-bad').style.width;
  document.querySelector('[data-tabs="itab"] [data-tab="plan"]').classList.toggle('live', !!run && run.state !== 'done');
  if (suites.length) {
    const order = new Map((S.catalog ? S.catalog.suites : []).map((s, i) => [s.name, [i + 1, s]]));
    sync($('i-plan'), suites, s => s.name, () => {
      const row = document.createElement('tr');
      row.innerHTML = '<td><span class="phase"></span><span class="order"></span></td><td><div class="file"></div><div class="desc"></div></td><td></td><td class="mono"></td><td class="num"></td><td class="num"></td>';
      return row;
    }, (row, s) => {
      const [index, info] = order.get(s.name) || [0, null];
      const cells = row.children;
      row.classList.toggle('active', s.state === 'running' || s.state === 'assigned');
      cells[0].firstChild.className = `phase ${s.serial ? 'serial' : 'parallel'}`;
      cells[0].firstChild.textContent = s.serial ? 'SERIAL' : 'PARALLEL';
      cells[0].lastChild.textContent = index ? `#${index}` : '';
      cells[1].firstChild.textContent = s.name;
      cells[1].lastChild.textContent = info ? `${info.bots}봇${info.extra_slot ? ' · extra' : ''}${info.labels.length ? ` · ${info.labels.join(', ')}` : ''}` : '';
      cells[2].className = `state ${SUITE_CLS[s.state]}`;
      cells[2].textContent = SUITE_WORD[s.state] || s.state;
      cells[3].textContent = s.agent_name || '—';
      cells[4].textContent = s.attempts;
      cells[5].textContent = s.seconds ? `${s.seconds}초` : '-';
    });
  } else {
    $('i-plan').innerHTML = statsRows([], 6);
  }

  $('i-tab-results').textContent = run ? packets.length : '';
  $('i-stats').innerHTML = statsRows(packets.map(r => `<tr><td class="mono">${esc(r.name)}</td><td class="num">${num(r.count)}</td><td class="num${r.failures ? ' bad' : ''}">${num(r.failures)}</td>
<td class="num">${num(r.avg)}</td><td class="num">${num(r.p50)}</td><td class="num">${num(r.p95)}</td><td class="num">${num(r.p99)}</td><td class="num">${num(r.max)}</td></tr>`).join(''), 8);

  $('i-tab-errors').textContent = failed.length || '';
  $('i-errors').innerHTML = failed.length ? failed.map(s => `<details><summary><span class="bad">${SUITE_WORD[s.state]}</span><span>${esc(s.name)}</span><span class="faint">${esc(s.agent_name || '-')}</span></summary><pre class="log">${esc(s.log || '로그 없음')}</pre></details>`).join('')
    : '<div class="empty">실패한 스위트가 없습니다</div>';

  if (S.catalog && S.catalog.revision !== state.revision)
    loadCatalog();
  updateIntegrationButtons();
}

function chart(id) {
  if (!window.echarts)
    return null;
  if (!S.charts[id])
    S.charts[id] = echarts.init($(id));
  return S.charts[id];
}

const AXIS = {
  animation: true,
  animationDuration: 600,
  animationDurationUpdate: TWEEN_MS,
  animationEasingUpdate: 'linear',
  grid: { left: 48, right: 40, top: 22, bottom: 20 },
  tooltip: {
    trigger: 'axis', backgroundColor: '#161b22', borderColor: '#30363d', textStyle: { color: '#e6edf3', fontSize: 13 },
    formatter: items => `${clock(items[0].value[0])}<br>${items.map(p => `${p.marker}${esc(p.seriesName)} ${num(p.value[1])}`).join('<br>')}`,
  },
  legend: { top: 0, right: 0, icon: 'rect', itemWidth: 10, itemHeight: 2, textStyle: { color: '#8d96a0', fontSize: 12 } },
  yAxis: { type: 'value', splitLine: { lineStyle: { color: '#21262d' } }, axisLabel: { color: '#6e7681', fontSize: 11 } },
};
const line = (name, data, color, extra) => Object.assign({ name, type: 'line', data, showSymbol: false, smooth: 0.3, lineStyle: { width: 1.4, color }, itemStyle: { color } }, extra || {});

function draw(prefix, timeline, ids) {
  const xAxis = {
    type: 'value', min: 'dataMin', max: 'dataMax', splitLine: { show: false }, axisLine: { lineStyle: { color: '#30363d' } }, axisTick: { show: false },
    axisLabel: { color: '#6e7681', fontSize: 11, formatter: clock, hideOverlap: true },
  };
  const perMinute = key => timeline.map((p, i) => {
    const back = timeline[Math.max(0, i - 15)];
    const span = p.t - back.t;
    return span > 0 ? Math.round((p[key] - back[key]) / span * 60) : 0;
  });
  const xy = values => values.map((v, i) => [timeline[i].t, v]);
  const field = key => timeline.map(p => [p.t, p[key]]);
  const sets = [
    [line('초당 요청', field('rps'), '#4493f8', { areaStyle: { color: 'rgba(56,139,253,0.08)' } }), line('초당 실패', field('fps'), '#f85149')],
    [line('p50', field('p50'), '#6e7681'), line('p95', field('p95'), '#4493f8'), line('p99', field('p99'), '#8d96a0', { lineStyle: { width: 1, type: 'dashed', color: '#8d96a0' } })],
    [line('접속 봇', field('bots'), '#4493f8', { areaStyle: { color: 'rgba(56,139,253,0.08)' } }), line('Agent', field('agents'), '#8d96a0', { yAxisIndex: 1, step: 'end', smooth: false })],
    [line('처치', xy(perMinute('kills')), '#4493f8'), line('사망', xy(perMinute('deaths')), '#8d96a0')],
  ];
  ids.forEach((id, i) => {
    const target = chart(id);
    if (!target)
      return;
    const yAxis = i === 2 ? [AXIS.yAxis, Object.assign({}, AXIS.yAxis, { splitLine: { show: false }, minInterval: 1 })] : AXIS.yAxis;
    target.setOption(Object.assign({}, AXIS, { xAxis, yAxis, series: sets[i] }));
  });
  return { perMinute };
}

function renderLoad() {
  const state = S.state;
  const agents = state.agents;
  const run = state.run && state.run.kind === 'load' ? state.run : null;
  const active = !!state.run && state.run.state !== 'done';
  const live = agents.filter(a => a.state !== 'lost');
  const timeline = run ? run.timeline : [];
  const last = timeline[timeline.length - 1] || {};

  count('l-agents', [live.length], whole);
  $('l-fleet').innerHTML = fleetSummary(agents);
  $('l-tab-agents').textContent = agents.length;
  const bots = agents.reduce((t, a) => t + a.bots, 0);
  count('l-bots', [bots, run ? run.users : 0], pair);
  $('l-meter').style.width = run && run.users ? `${Math.min(100, bots / run.users * 100)}%` : '0%';
  const working = agents.filter(a => a.state === 'run');
  $('l-bots-sub').textContent = working.length ? `Agent당 평균 ${num(Math.round(bots / working.length))} · 최소 ${num(Math.min(...working.map(a => a.bots)))} · 최대 ${num(Math.max(...working.map(a => a.bots)))}` : '-';
  count('l-rps', [last.rps || 0], whole);
  $('l-rps-sub').textContent = timeline.length ? `최근 최고 ${num(Math.max(...timeline.map(p => p.rps)))}` : '-';
  const packets = run ? run.stats.filter(r => kind(r.name) === 'packet') : [];
  const total = packets.reduce((t, r) => t + r.count, 0), fails = packets.reduce((t, r) => t + r.failures, 0);
  count('l-fail', [total ? fails / total * 100 : 0], v => `${v[0].toFixed(2)}%`);
  $('l-fail-sub').textContent = `초당 실패 ${num(last.fps)} · 누적 ${num(fails)}`;
  count('l-p95', [last.p95], v => `${num(Math.round(v[0]))} ms`);
  $('l-p95-sub').textContent = last.p50 !== undefined ? `p50 ${num(last.p50)} · p99 ${num(last.p99)}` : '-';

  if (S.view === 'load') {
    const { perMinute } = draw('l', timeline, ['c1', 'c2', 'c3', 'c4']);
    const kills = perMinute('kills'), deaths = perMinute('deaths');
    count('l-kills', [kills[kills.length - 1] || 0, deaths[deaths.length - 1] || 0], pair);
    const exp = perMinute('exp');
    $('l-kills-sub').textContent = `분당 경험치 ${num(exp[exp.length - 1])}`;
    count('c1-v', [last.rps || 0], whole);
    count('c2-v', [last.p95], v => `${num(Math.round(v[0]))} ms`);
    count('c3-v', [last.bots || 0], whole);
    count('c4-v', [kills[kills.length - 1] || 0], whole);
  }

  const rows = run ? run.stats.slice().sort((a, b) => b.count - a.count) : [];
  $('l-tab-requests').textContent = rows.length || '';
  $('l-stats').innerHTML = statsRows(rows.map(r => {
    const timed = r.timed > 0;
    return `<tr><td class="mono">${esc(r.name)}</td><td><span class="tag">${kind(r.name)}</span></td><td class="num">${num(r.count)}</td><td class="num${r.failures ? ' bad' : ''}">${num(r.failures)}</td>
<td class="num">${timed ? num(r.p50) : '-'}</td><td class="num">${timed ? num(r.p95) : '-'}</td><td class="num">${timed ? num(r.p99) : '-'}</td><td class="num">${r.rps}</td></tr>`;
  }).join(''), 8);

  const errors = rows.filter(r => r.failures > 0).sort((a, b) => b.failures - a.failures);
  $('l-tab-errors').textContent = errors.length || '';
  $('l-errors').innerHTML = statsRows(errors.map(r => `<tr><td class="mono">${esc(r.name)}</td><td class="num bad">${num(r.failures)}</td><td class="num">${num(r.count)}</td><td class="num">${(r.failures / r.count * 100).toFixed(1)}%</td></tr>`).join(''), 4);

  const goals = run ? Object.entries(run.goals).sort((a, b) => b[1] - a[1]) : [];
  const peak = Math.max(1, ...goals.map(g => g[1]));
  if (goals.length) {
    sync($('l-goals'), goals, g => g[0], ([name]) => {
      const row = document.createElement('div');
      row.innerHTML = `<span>${esc(name)}</span><span><b></b></span><span class="num"></span>`;
      return row;
    }, (row, [, value]) => {
      row.querySelector('b').style.width = `${value / peak * 100}%`;
      row.lastElementChild.textContent = num(value);
    });
  } else {
    $('l-goals').innerHTML = '<div class="empty">아직 기록이 없습니다</div>';
  }

  const counts = working.map(a => a.bots);
  if (counts.length) {
    const low = Math.min(...counts), high = Math.max(...counts), width = Math.max(1, Math.ceil((high - low + 1) / 12));
    const buckets = Array.from({ length: Math.ceil((high - low + 1) / width) }, () => 0);
    counts.forEach(c => buckets[Math.floor((c - low) / width)]++);
    const top = Math.max(...buckets);
    sync($('l-dist'), buckets, (v, i) => i, () => document.createElement('i'), (bar, v, i) => {
      bar.style.height = `${Math.max(4, Math.round(v / top * 100))}%`;
      bar.title = `${low + i * width}~${low + (i + 1) * width - 1}봇 · ${v}대`;
    });
    $('l-dist-axis').innerHTML = `<span>${num(low)}봇</span><span>${num(high)}봇</span>`;
  } else {
    sync($('l-dist'), [], (v, i) => i, null, null);
    $('l-dist-axis').innerHTML = '<span>실행 중인 Agent가 없습니다</span>';
  }
  S.fleets.la(agents);

  const running = run && run.state === 'running';
  if (run && S.loadRun !== run.id) {
    S.loadRun = run.id;
    $('l-users').value = run.users;
    $('l-rate').value = run.rate;
    $('l-cap').value = run.cap;
    $('l-duration').value = run.duration;
    $('l-offset').value = run.offset;
    $('l-client').value = run.client;
    $('l-target').value = run.target || '';
  }
  ['l-rate', 'l-cap', 'l-duration', 'l-offset', 'l-client', 'l-target'].forEach(id => $(id).disabled = !!running);
  $('l-start').textContent = running ? '봇 수 적용' : '시작';
  $('l-start').disabled = active && !running;
  $('l-stop').disabled = !running;
}

function tick() {
  const c = S.clock;
  const run = S.state && S.state.run;
  const elapsed = c.live ? Math.min(c.base + (performance.now() - c.at) / 1000, c.base + 3) : c.base;
  c.shown = c.live ? Math.max(c.shown, elapsed) : elapsed;
  const integration = run && run.kind === 'integration', load = run && run.kind === 'load';
  $('i-elapsed').textContent = integration ? clock(c.shown) : '-';
  $('i-status').textContent = integration ? `${run.state.toUpperCase()} · ${clock(c.shown)}` : 'IDLE';
  $('l-status').firstChild.textContent = load ? `${run.state.toUpperCase()} · ${clock(c.shown)}${c.duration ? ' / ' + clock(c.duration * 60) : ''}` : 'IDLE';
  $('l-time').style.width = load && c.duration ? `${Math.min(100, c.shown / (c.duration * 60) * 100)}%` : '0%';
}

function render() {
  if (!S.state)
    return;
  const run = S.state.run;
  const c = S.clock;
  if (run) {
    if (c.id !== run.id)
      c.shown = 0;
    Object.assign(c, { id: run.id, base: run.elapsed, at: performance.now(), live: run.state !== 'done', duration: run.duration || 0 });
  } else {
    Object.assign(c, { id: null, base: 0, shown: 0, live: false, duration: 0 });
  }
  renderIntegration();
  renderLoad();
  tick();
}

async function loadCatalog() {
  try {
    S.catalog = await api('/api/suites');
    renderCatalog();
  } catch (error) {
    notice('i-notice', `스위트 목록을 읽지 못했습니다: ${error.message}`, true);
  }
}

async function loadTargets() {
  try {
    const { targets } = await api('/api/targets');
    document.querySelectorAll('[data-targets]').forEach(select => {
      const chosen = select.value;
      select.innerHTML = '<option value="">config 기본</option>' + targets.map(t =>
        `<option value="${esc(t.address)}">${esc(t.label || t.name)} · ${esc(t.address)}</option>`).join('');
      if ([...select.options].some(o => o.value === chosen))
        select.value = chosen;
    });
  } catch (error) {
    notice('l-notice', `접속 대상 목록을 읽지 못했습니다: ${error.message}`, true);
  }
}

async function reload(button) {
  button.disabled = true;
  try {
    const result = await api('/api/reload', {});
    const broken = result.files.filter(f => f.error);
    const text = `다시 읽었습니다 · 스위트 ${result.suites}개${broken.length ? ' · 오류: ' + broken.map(f => `${f.path} (${f.error})`).join(', ') : ''}`;
    notice('i-notice', text, broken.length > 0);
    notice('l-notice', text, broken.length > 0);
    await loadCatalog();
    await loadTargets();
  } catch (error) {
    notice('i-notice', error.message, true);
    notice('l-notice', error.message, true);
  }
  updateIntegrationButtons();
}

async function loadHistory() {
  try {
    const runs = await api('/api/runs');
    $('h-runs').innerHTML = runs.length ? runs.map(r => `<tr data-run="${esc(r.id)}"><td class="mono">${esc(r.id)}</td><td><span class="tag">${r.kind === 'load' ? '부하' : '통합'}</span></td>
<td>${local(r.started_at)}</td><td class="num">${clock(r.elapsed)}</td><td>${r.kind === 'load' ? `요청 ${num(r.requests)} · 실패 ${num(r.failures)}` : `<span class="good">통과 ${r.passed}</span> · <span class="${r.failed ? 'bad' : 'faint'}">실패 ${r.failed}</span>`}</td></tr>`).join('')
      : '<tr><td colspan="5" class="empty">끝난 실행이 없습니다</td></tr>';
  } catch (error) {
    $('h-runs').innerHTML = `<tr><td colspan="5" class="empty bad">${esc(error.message)}</td></tr>`;
  }
}

async function showRun(id) {
  $('h-runs').querySelectorAll('tr').forEach(r => r.classList.toggle('on', r.dataset.run === id));
  const run = await api(`/api/runs/${encodeURIComponent(id)}`);
  $('h-detail').hidden = false;
  $('h-title').textContent = run.id;
  $('h-sub').textContent = `${local(run.started_at)} ~ ${local(run.finished_at)} · ${clock(run.elapsed)}${run.kind === 'load' ? ` · 봇 ${num(run.users)}` : ''}`;
  $('h-files').innerHTML = ['summary.json', 'timeline.jsonl', 'stats.csv'].map(f => `<a href="/api/runs/${encodeURIComponent(run.id)}/${f}">${f}</a>`).join('');
  $('h-suites-wrap').hidden = !run.suites.length;
  $('h-suites').innerHTML = run.suites.map(s => `<tr><td class="file">${esc(s.name)}</td><td class="${SUITE_CLS[s.state]}">${SUITE_WORD[s.state] || s.state}</td><td class="mono">${esc(s.agent_name || '-')}</td><td class="num">${s.attempts}</td><td class="num">${s.seconds ? s.seconds + '초' : '-'}</td></tr>`).join('');
  $('h-stats').innerHTML = statsRows(run.stats.map(r => `<tr><td class="mono">${esc(r.name)}</td><td class="num">${num(r.count)}</td><td class="num${r.failures ? ' bad' : ''}">${num(r.failures)}</td>
<td class="num">${r.timed ? num(r.p50) : '-'}</td><td class="num">${r.timed ? num(r.p95) : '-'}</td><td class="num">${r.timed ? num(r.p99) : '-'}</td><td class="num">${r.rps}</td></tr>`).join(''), 7);
  draw('h', run.timeline || [], ['h1', 'h2']);
  const from = scrollY;
  const to = Math.min($('h-detail').getBoundingClientRect().top + scrollY - 16, document.documentElement.scrollHeight - innerHeight);
  const start = performance.now();
  const step = now => {
    const p = Math.min(1, (now - start) / 450);
    scrollTo(0, from + (to - from) * (1 - (1 - p) ** 3));
    if (p < 1)
      requestAnimationFrame(step);
  };
  requestAnimationFrame(step);
}

function show(view) {
  S.view = view;
  document.querySelectorAll('.view').forEach(v => v.hidden = v.id !== view);
  document.querySelectorAll('.views a').forEach(a => a.classList.toggle('on', a.dataset.view === view));
  store.set('fb.cluster.view', view);
  if (view === 'history')
    loadHistory();
  render();
  Object.values(S.charts).forEach(c => c.resize());
}

function connect() {
  const socket = new WebSocket(`${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/ws/dashboard`);
  socket.onopen = () => {
    $('online').textContent = 'Master 온라인';
    $('online').classList.remove('off');
  };
  socket.onmessage = event => {
    const message = JSON.parse(event.data);
    if (message.type !== 'state')
      return;
    S.state = message;
    render();
  };
  socket.onclose = () => {
    $('online').textContent = 'Master 연결 끊김';
    $('online').classList.add('off');
    setTimeout(connect, 2000);
  };
}

function bind() {
  document.querySelectorAll('[data-tabs]').forEach(bar => {
    const group = bar.dataset.tabs;
    bar.addEventListener('click', e => {
      const button = e.target.closest('[data-tab]');
      if (!button)
        return;
      bar.querySelectorAll('[data-tab]').forEach(x => x.setAttribute('aria-selected', String(x === button)));
      document.querySelectorAll(`[data-panel^="${group}:"]`).forEach(p => p.hidden = p.dataset.panel !== `${group}:${button.dataset.tab}`);
    });
  });
  document.querySelectorAll('[data-fleet]').forEach(host => S.fleets[host.dataset.fleet] = fleet(host, host.dataset.mode));
  document.querySelectorAll('.views a').forEach(a => a.addEventListener('click', () => show(a.dataset.view)));

  const toggle = (names, on) => {
    names.forEach(n => on ? S.selected.add(n) : S.selected.delete(n));
    store.set('fb.cluster.selected', [...S.selected]);
    renderCatalog();
  };
  $('i-suites').addEventListener('click', e => {
    const row = e.target.closest('[data-suite]');
    if (row)
      toggle([row.dataset.suite], !S.selected.has(row.dataset.suite));
  });
  $('i-labels').addEventListener('click', e => {
    const button = e.target.closest('[data-label]');
    if (!button || !S.catalog)
      return;
    const label = button.dataset.label;
    const names = S.catalog.suites.filter(s => s.labels.includes(label) || (label === 'serial' && s.serial)).map(s => s.name);
    toggle(names, !names.every(n => S.selected.has(n)));
  });
  $('i-all').addEventListener('click', () => S.catalog && toggle(S.catalog.suites.map(s => s.name), true));
  $('i-none').addEventListener('click', () => S.catalog && toggle([...S.selected], false));
  $('i-failed').addEventListener('click', () => {
    const run = S.state && S.state.run;
    if (!run || run.kind !== 'integration')
      return;
    S.selected = new Set(run.suites.filter(s => s.state === 'failed' || s.state === 'lost').map(s => s.name));
    store.set('fb.cluster.selected', [...S.selected]);
    renderCatalog();
  });
  $('i-search').addEventListener('input', e => { S.search = e.target.value; renderCatalog(); });

  $('i-start').addEventListener('click', async () => {
    $('i-start').disabled = true;
    try {
      const order = S.catalog.suites.map(s => s.name).filter(n => S.selected.has(n));
      await api('/api/runs', { kind: 'integration', suites: order, client: $('i-client').value, target: $('i-target').value, retry: Number($('i-retry').value) });
      notice('i-notice', '');
      document.querySelector('[data-tabs="itab"] [data-tab="plan"]').click();
    } catch (error) {
      notice('i-notice', error.message, true);
    }
    updateIntegrationButtons();
  });
  $('i-stop').addEventListener('click', () => api('/api/runs/current/stop', {}).catch(error => notice('i-notice', error.message, true)));
  $('l-start').addEventListener('click', async () => {
    const run = S.state && S.state.run;
    try {
      if (run && run.kind === 'load' && run.state === 'running') {
        await api('/api/runs/current/scale', { users: Number($('l-users').value) });
      } else {
        await api('/api/runs', {
          kind: 'load', users: Number($('l-users').value), rate: Number($('l-rate').value), cap: Number($('l-cap').value),
          duration: Number($('l-duration').value), offset: Number($('l-offset').value), client: $('l-client').value,
          target: $('l-target').value,
        });
      }
      notice('l-notice', '');
    } catch (error) {
      notice('l-notice', error.message, true);
    }
  });
  $('l-stop').addEventListener('click', () => api('/api/runs/current/stop', {}).catch(error => notice('l-notice', error.message, true)));
  $('i-reload').addEventListener('click', () => reload($('i-reload')));
  $('l-reload').addEventListener('click', () => reload($('l-reload')));
  $('h-runs').addEventListener('click', e => {
    const row = e.target.closest('[data-run]');
    if (row)
      showRun(row.dataset.run).catch(error => alert(error.message));
  });
  window.addEventListener('resize', () => Object.values(S.charts).forEach(c => c.resize()));
}

bind();
setInterval(tick, 250);
show(['integration', 'load', 'history'].includes(location.hash.slice(1)) ? location.hash.slice(1) : store.get('fb.cluster.view', 'integration'));
loadCatalog();
loadTargets();
connect();
