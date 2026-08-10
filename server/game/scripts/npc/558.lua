-- npc: 견우

return {
    on_click = function(me, npc)
        if me:gender() == GENDER.FEMALE then
            local button = me:dialog(npc, '여자분들은 직녀에게 말을 걸어보세요.', { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        local button = me:dialog(npc, string.format('%s님이 아니었으면 이번에 저는 직녀를 못 만날뻔 했네요... 정말 감사합니다. 이 은혜를 어떻게 갚아야 할지...', me:name()), { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, '도와주신 분들 중 남자분들은 제가 사용하는 지팡이를 드리도록 하겠습니다. 천상의 가축들을 다루는데 사용하는 것이라 매우 단단한 것이랍니다.', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, '정말 감사합니다... 안녕히 돌아가세요..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if me:mkitem('견우의지팡이', 1) == nil then
            return
        end

        local village = game_map(25228) or game_map('세시마을')
        if village ~= nil then
            me:map(village, math.random(52, 59), math.random(46, 50))
        end
    end
}
