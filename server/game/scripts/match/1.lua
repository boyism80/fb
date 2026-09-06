-- match: MATCH_1 (PvP)

local MATCH_DURATION_SECONDS = 600

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
        if match:state() ~= 'playing' then
            return
        end

        local team = match:team(me)
        if team == nil then
            match:finish(5)
            return
        end

        for _, ch in ipairs(team:members()) do
            if ch:state() ~= STATE.GHOST then
                return
            end
        end

        match:finish(5)
    end,

    on_end = function(match, me)
        me:timer(0, TIMER_TYPE.OFF)
        me:message('매치가 종료되었습니다.', MESSAGE_TYPE.STATE)
    end
}
