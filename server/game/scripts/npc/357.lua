-- npc: 백세인
local quest = require('lib.quest')

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "안녕하세요? 저는 백세인입니다.", { prev = false, next = true })
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

        if me:has_items("찌그러진냄비", 1) then
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
            return
        end

        button = me:dialog(npc, "몽연이가 청소를 한다구요?", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "작년 추석에 송편 만들다가 냄비를 떨어뜨려서 냄비가 찌그러졌는데.. 잘됐네요! 나중에 선릉이한테 냄비 새로 만들어 달래야지~", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "감사합니다!!", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        me:mkitem("찌그러진냄비", 1)
    end
}
