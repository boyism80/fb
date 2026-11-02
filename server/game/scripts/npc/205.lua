-- npc: 부여-일본섬뱃사공

return {
    on_click = function(me, npc)
        local t = datetime()
        local minute = t.minute
        if minute >= 40 then
            -- Arrival window is time-gated: do not allow QUIT to skip disembark.
            me:dialog(npc, '부여입니다. 안녕히 가십시오.', { prev = false, next = false })
            local map = id2map(1822)
            if map ~= nil then
                me:map(map, { 54, 14 })
            end
        elseif minute >= 10 then
            -- Arrival window is time-gated: do not allow QUIT to skip disembark.
            me:dialog(npc, '일본입니다. 안녕히 가십시오.', { prev = false, next = false })
            local map = id2map(1822)
            if map ~= nil then
                me:map(map, { 194, 14 })
            end
        else
            me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', { prev = false, next = false })
        end
    end
}
