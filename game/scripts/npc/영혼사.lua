function on_interact(me, npc)

::ROUTINE_1::
    local index, button = npc:list(me, '그래도 버티고 서서 도대체 원하는 것이 뭐냐?', {'힘올리기', '지력올리기', '민첩올리기', '체력사기', '마력사기', '성형', '성전환'}, false)
    if index == 5 then
        index, button = npc:list(me, '지금의 모습을 벗어나 새로운 얼굴을 갖기를 원하고 있나?', {'예', '아니오'})
        if index ~= 0 then
            return
        end

        index, button = npc:list(me, '사회에서 물의를 빚고 숨어서 지내려는 것은 아니겠지?', {'아닙니다.', '예. 잠시 숨어서....'})
        if index ~= 0 then
            return
        end

        index, button = npc:list(me, '수술을 하려면 말일세, 백만전의 금이 필요하다네.. 가지고 있는가?', {'예', '아니오'})
        if index ~= 0 then
            return
        end

::ROUTINE_2::
        button = npc:dialog(me, '지금부터 차례로 자네가 갖을 수 있는 모습을 보여주도록 하겠다. 수술이 그리 쉽지는 않고', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = npc:dialog(me, '원하는 모습이 보이면 바로 예를 선택하도록. 현재 네가 가질 수 있는 얼굴은 101개가 있다. 이전이나 다음을 선택해도 얼굴이 바뀌지 않으면 마지막 모습이니 그리 알도록.', true, true)
        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_2
        end

        if button == DIALOG_RESULT_QUIT then
            return
        end

        local face = 0
        while true do
            index, button = npc:list(me, '다음에 또 수술을 해 줄 수는 있지만, 수술을 계속한다고 결코 좋아지지는 않는다. 이 모습을 선택하겠느냐?', {'선택', '이전 모습으로', '다음 모습으로', '수술 포기'}, false, me, face)
            if index == nil then
                return
            end

            if index == 0 then
                if npc:dialog(me, '그럼 네 얼굴을 그 모습을 고쳐주도록 하지.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                local money = me:money()
                if money < 1000000 then
                    if npc:dialog(me, '백만원이 없잖아 임마', false, true) == DIALOG_RESULT_QUIT then
                        return
                    end
                    goto ROUTINE_1
                end

                me:look(face)
                me:money(money - 1000000)
                if npc:dialog(me, '새 얼굴이 마음에 드는가? 맘에 안들어도 다시 해 줄 수는 없네.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                if npc:dialog(me, '나에게 더 도움을 받을 일은 이제 없겠지?.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                goto ROUTINE_1
            elseif index == 1 then
                face = (face - 1 + 102) % 102
            elseif index == 2 then
                face = (face + 1) % 102
            elseif index == 3 then
                npc:dialog(me, '수술 포기')
                return
            end
        end
    end
end