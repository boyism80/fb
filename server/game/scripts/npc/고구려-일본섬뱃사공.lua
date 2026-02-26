-- Boatman (뱃사공) for 고구려-일본 ferry. Disembark: 고구려-일본선착장 (54,14) or 일본 동쪽해안가 (1,44).
-- Time: minute 40-59 = departure port (고구려); 20-40 = arrival port (일본); else sailing.

function NPC_207(me, npc)
    local t = datetime()
    local minute = t.minute
    if minute >= 40 and minute <= 59 then
        local btn = me:dialog(npc, '고구려입니다. 안녕히 가십시오.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local map = name2map('고구려-일본선착장')
        if map ~= nil then
            me:map(map, {54, 14})
        end
    elseif minute >= 0 and minute <= 19 then
        me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', false, true)
    elseif minute >= 20 and minute <= 40 then
        local btn = me:dialog(npc, '일본입니다. 안녕히 가십시오.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local map = name2map('동쪽해안가')
        if map ~= nil then
            me:map(map, {1, 44})
        end
    else
        me:dialog(npc, '지금 항해중입니다. 곧 도착할 예정이오니 잠시만 더 기다려 주시기 바랍니다.', false, true)
    end
end
