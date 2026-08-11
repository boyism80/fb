-- npc: 직녀

return {
    on_click = function(me, npc)
        if me:gender() == GENDER.MALE then
            local button = me:dialog(npc, '남자분들은 견우님에게 말을 걸어보세요.', { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        local button = me:dialog(npc, string.format('%s님이 아니었으면 이번에 저는 견우님을 못 만날뻔 했네요... 정말 감사합니다. 이 은혜를 어떻게 갚아야 할지...', me:name()), { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, '도와주신 분들 중 여자분들은 제가 만든 옷을 드릴께요.. 제 성의이니 꼭 받아주세요..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        button = me:dialog(npc, '정말 감사합니다... 안녕히 돌아가세요..', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if me:mkitem('직녀의옷', 1) == nil then
            return
        end

        local village = id2map(25228) or name2map('세시마을')
        if village ~= nil then
            me:map(village, math.random(52, 59), math.random(46, 50))
        end
    end
}
