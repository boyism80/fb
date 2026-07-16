-- npc: 정어언
local quest = require('lib.quest')

function NPC_356(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 정어언입니다.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if property("sesi_rightnow") ~= 2 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    if q:step() < 1 then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    if me:has_items("널", 1) then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end

    button = me:dialog(npc, "몽연이가 청소를 한다구요? 그럼 저는 널뛰기 판자를 부탁드릴께요..", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "이 널은 세시마을에 있는 여자애들이 명절때마다 쓰던 널이랍니다. 이젠 너무 낡아서 새로 장만해야겠네요.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 몽연이에게 잘 전해주세요!", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("널", 1)
end
