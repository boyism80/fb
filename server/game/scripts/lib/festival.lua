local M = {}

local FESTIVALS = {
    ['설날'] = { month = 1, day_begin = 1, day_end = 15 },
    ['중화절'] = { month = 2, day_begin = 1, day_end = 1 },
    ['삼짇날'] = { month = 3, day_begin = 3, day_end = 3 },
    ['석가탄신일'] = { month = 4, day_begin = 8, day_end = 8 },
    ['단오'] = { month = 5, day_begin = 5, day_end = 5 },
    ['유두'] = { month = 6, day_begin = 15, day_end = 15 },
    ['칠석'] = { month = 7, day_begin = 7, day_end = 7 },
    ['추석'] = { month = 8, day_begin = 15, day_end = 15 },
    ['동지'] = { month = 11, day_begin = 1, day_end = 30 },
}

-- In-memory kill penalty for 꿩 (석가탄신일). Keyed by character name.
local pheasant_penalty_until = {}

function M.is(name)
    local e = FESTIVALS[name]
    if e == nil then
        return false
    end

    local lunar = to_lunar(datetime())
    if lunar == nil then
        return false
    end

    if lunar.leap then
        return false
    end

    if lunar.month ~= e.month then
        return false
    end
    if lunar.day < e.day_begin or lunar.day > e.day_end then
        return false
    end
    return true
end

-- 칠성당 입장/고사: 0,3,6,9,12,15,18,21
function M.is_triple_hour(hour)
    return (hour % 3) == 0
end

function M.set_pheasant_kill_penalty(me, seconds)
    if me == nil then
        return
    end
    local ts = now()
    if ts == nil or ts <= 0 then
        return
    end
    pheasant_penalty_until[me:name()] = ts + (seconds or 3600)
end

function M.pheasant_kill_penalty_remaining(me)
    if me == nil then
        return 0
    end
    local until_ts = pheasant_penalty_until[me:name()]
    if until_ts == nil then
        return 0
    end
    local ts = now()
    if ts == nil or ts <= 0 then
        return 0
    end
    if until_ts <= ts then
        pheasant_penalty_until[me:name()] = nil
        return 0
    end
    return until_ts - ts
end

function M.wash_hair(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local sel, list_btn = me:list(me, "머리를 감으시겠습니까?", {
        "네. 감을래요.",
        "아니오.",
    }, { prev = false })
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel ~= 1 then
        return
    end

    if me:isbuff("반짝반짝") then
        me:dialog(me, "머리감은지 얼마 안됐는데 또 감으시려구요? 머리 너무 자주 감으시면 안좋아요...", { prev = false, next = false })
        return
    end

    me:message("머리를 감았습니다.")
    me:buff("반짝반짝", 185)
end

return M
