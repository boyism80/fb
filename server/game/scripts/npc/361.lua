-- npc: 백주연
local quest = require('lib.quest')

function NPC_361(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 백주연입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if property("sesi_rightnow") ~= 2 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if q:step() < 1 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if me:has_items("실패", 1) then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    button = me:dialog(npc, "벌써 몽연오빠가 청소하는 때가 되었나보네요.. 실패를 가져오라고 했다구요? 잠시만요.....", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "................여기있네요.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 행복한 봄날 되세요!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("실패", 1)
end
