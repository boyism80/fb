local event = require('lib.event')

local M = {}

function M.is(name)
    return event.is(name)
end

-- Lunar calendar year for annual festival quest resets.
function M.lunar_year()
    local lunar = to_lunar(datetime())
    if lunar == nil then
        return 0
    end
    return lunar.year or 0
end

-- If the quest was completed (or stamped) in a previous lunar year, clear it for replay.
-- Param convention: leading number is lunar year (e.g. "2026" or "2026:0:0:0").
function M.reset(q)
    if q == nil then
        return M.lunar_year()
    end

    local year = M.lunar_year()
    local p = q:param() or ''
    local stored = tonumber(string.match(p, '^(%d+)'))

    if q:completed() then
        if stored == nil or stored < year then
            q:resume()
            q:step(0)
            q:progress(0)
            q:param('')
        end
        return year
    end

    if stored ~= nil and stored < year then
        q:step(0)
        q:progress(0)
        q:param('')
    elseif stored == nil and (q:step() > 0 or q:progress() > 0) then
        -- Legacy in-progress without a year stamp: bind to this lunar year.
        q:param(tostring(year))
    end
    return year
end

-- Record the current lunar year as the completion stamp (call before/with complete).
function M.mark_completed_year(q)
    if q == nil then
        return
    end
    M.stamp_lunar_year(q)
end

-- Stamp or refresh the leading lunar year on an in-progress param (keeps "YEAR:rest").
function M.stamp_lunar_year(q)
    if q == nil then
        return
    end
    local year = M.lunar_year()
    local p = q:param() or ''
    local rest = string.match(p, '^%d+:(.*)$')
    if rest ~= nil then
        q:param(string.format('%d:%s', year, rest))
    else
        q:param(tostring(year))
    end
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
    local quest = require('lib.quest')
    local q = quest.get_annual(me, quest.QUEST_BUDDHA_LANTERN) or me:start_quest(quest.QUEST_BUDDHA_LANTERN)
    if q == nil then
        return
    end
    q:param(string.format('%d:%d', M.lunar_year(), ts + (seconds or 3600)))
end

function M.pheasant_kill_penalty_remaining(me)
    if me == nil then
        return 0
    end
    local quest = require('lib.quest')
    local q = quest.get_annual(me, quest.QUEST_BUDDHA_LANTERN)
    if q == nil then
        return 0
    end
    local until_ts = tonumber(string.match(q:param() or '', '^%d+:(%d+)$'))
    if until_ts == nil then
        return 0
    end
    local ts = now()
    if ts == nil or ts <= 0 then
        return 0
    end
    if until_ts <= ts then
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
