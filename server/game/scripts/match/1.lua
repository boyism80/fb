-- match: MATCH_1 (PvP)

local MATCH_DURATION_SECONDS = 600

local function team_wiped(team)
    for _, ch in ipairs(team:members()) do
        if ch:state() ~= STATE.GHOST then
            return false
        end
    end
    return true
end

local function finish_if_decided(match)
    if match:state() ~= 'playing' then
        return
    end

    for _, team in ipairs(match:teams()) do
        if team_wiped(team) then
            match:finish(5)
            return
        end
    end
end

return {
    on_join = function(match, me)
        if match:state() == 'waiting' and match:member_count() == 1 then
            match:wait(30)
        end
    end,

    on_playing = function(match, me)
        me:timer(MATCH_DURATION_SECONDS, TIMER_TYPE.INCREASE)
        me:message('매치가 시작되었습니다.', MESSAGE_TYPE.STATE)
        match:duration(MATCH_DURATION_SECONDS, 5)
    end,

    on_death = function(match, me, killer)
        if match:state() == 'playing' and match:team(me) == nil then
            match:finish(5)
            return
        end
        finish_if_decided(match)
    end,

    on_leave = function(match)
        finish_if_decided(match)
    end,

    on_end = function(match, me)
        me:timer(0, TIMER_TYPE.OFF)
        me:message('매치가 종료되었습니다.', MESSAGE_TYPE.STATE)
    end
}
