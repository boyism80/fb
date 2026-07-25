-- npc: 부여-일본섬뱃사공

return {
    ON_CLICK = function(me, npc)
        local t = datetime()
        local minute = t.minute
        if minute >= 35 and minute <= 59 then
            local btn = me:dialog(npc, '부여입니다. 안녕히 가십시오.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local map = name2map('부여-일본선착장')
            if map ~= nil then
                me:map(map, {31, 1})
            end
        elseif minute >= 0 and minute <= 3 then
            local btn = me:dialog(npc, '일본입니다. 안녕히 가십시오.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local map = name2map('동쪽해안가')
            if map ~= nil then
                me:map(map, {1, 44})
            end
        else
            me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', { prev = false, next = true })
        end
    end
}
