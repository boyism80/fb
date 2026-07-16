-- npc: 사냥이벤트도우미
local function run_hunt_event_entry(me, npc)
    local btn = me:dialog(npc, "사냥이벤트에 입장하게 되면, 9시가 되는 순간까지 이벤트에 참여하실 수 있습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "또한 입장한 뒤 귀환을 하거나 접속을 종료하시는 경우 다시 입장을 하실 수 없습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, "위 사항에 대해서 모두 숙지를 하신 뒤 입장을 해 주시길 바라겠습니다.", { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "사냥이벤트에 입장하기 위해서는 '노란비서'가 필요합니다. 지금 입장하시겠습니까?", { "예", "아니오" }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 2 then
        me:dialog(npc, "안녕히 가십시오.", { prev = false, next = false })
        return
    end

    if not me:has_items("노란비서", 1) then
        me:dialog(npc, "만일의 사태를 대비하기 위해 노란비서를 지참해 주시기 바랍니다.", { prev = false, next = false })
        return
    end
    if not me:rmitem("노란비서", 1, ITEM_DELETE_TYPE.REDUCE) then
        return
    end
    local event_map = name2map("사냥이벤트1")
    if event_map then
        me:map(event_map, math.random(2, 199), math.random(2, 199))
    else
        local fallback = name2map("부여성")
        if fallback then
            me:map(fallback, math.random(70, 80), math.random(138, 145))
        else
            me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
        end
    end
end

local function run_hunt_event_admin(me, npc)
    run_hunt_event_entry(me, npc)
end

local function run_hunt_event_user(me, npc)
    local dt = datetime()
    local hour_ok = (dt.hour == nil or dt.hour == 20)
    local in_window = (dt.month == 2 and (dt.day == 4 or dt.day == 5 or dt.day == 6) and hour_ok)
    if not in_window then
        me:dialog(npc, "입장 가능한 시각은 20:00 부터 21:00까지 입니다.", { prev = false, next = false })
        return
    end
    if me:level() < 99 then
        me:dialog(npc, "무분별한 세컨아이디 참가를 막기 위해, 레벨 99 이상인 유저만 입장이 가능합니다.", { prev = false, next = false })
        return
    end
    run_hunt_event_entry(me, npc)
end

function NPC_456(me, npc)
    local is_admin = (me:role() >= ROLE.ADMIN)
    if is_admin then
        run_hunt_event_admin(me, npc)
    else
        run_hunt_event_user(me, npc)
    end
end