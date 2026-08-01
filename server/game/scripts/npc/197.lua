-- npc: 고구려-섬뱃사공

return {
    on_click = function(me, npc)
        local t = datetime()
        local minute = t.minute
        if minute >= 40 and minute <= 59 then
            local btn = me:dialog(npc, '고구려입니다. 안녕히 가십시오.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local map = name2map('고구려-고균도선착장')
            if map ~= nil then
                me:map(map, {14, 26})
            end
        elseif minute >= 0 and minute <= 19 then
            me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', { prev = false, next = true })
        elseif minute >= 20 and minute <= 40 then
            local btn = me:dialog(npc, '고균도입니다. 안녕히 가십시오.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local map = name2map('고구려-고균도선착장')
            if map ~= nil then
                me:map(map, {14, 240})
            end
        else
            me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', { prev = false, next = true })
        end
    end
}
