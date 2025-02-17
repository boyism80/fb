function on_interact(me, npc)

::ROOT::
    local index, button = npc:list(me, '그래도 버티고 서서 도대체 원하는 것이 뭐냐?', {'힘올리기', '지력올리기', '민첩올리기', '체력사기', '마력사기', '성형', '성전환'}, false)
    if index == 3 then
::ROUTINE_BUY_HP_1::
        button = npc:dialog(me, '체력을 사기 전에 명심하게.. 지금 착용하고 있는 모든 아이템을 벗은 후, 체력을 사기 바라네.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

::ROUTINE_BUY_HP_2::
        button = npc:dialog(me, '만약, 내 말을 듣지 않고, 체력을 사서 일어나는 불이익에 대해서는 절대 책임을 질 수 없네!!', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_1
        end

::ROUTINE_BUY_HP_3::
        button = npc:dialog(me, '경험치로 체력을 사고 싶다고.... 늘 이렇게 경험치를 팔고는 여기 저기 가서 잘못 팔았다고 후회를 할 것이거늘... 쯧쯧... 어디 레벨이나 되는지 한 번 보자.', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_2
        end

        button = npc:dialog(me, '레벨은 충분하고...', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_3
        end

        local step = npc:input(me, '그래 체력을 몇 단계나 올리려고? 체력은 한 단계가 50이라는 것은 알고 있겠지?', '예,', '단계를 올리고 싶습니다.')
        local exp = step * 10000000
        local my_exp = me:exp()
        if my_exp < exp then
            npc:dialog(me,  '경험치가 모자람')
            goto ROOT
        end

        me:exp(my_exp - exp)
        me:base_hp(me:base_hp() + (50 * step))

::ROUTINE_BUY_HP_5::
        button = npc:dialog(me, string.format('자네의 경험치로 체력을 %d단계 올렸으니 그리 알고 돌아가도록 해.', step), false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

::ROUTINE_BUY_HP_6::
        button = npc:dialog(me, string.format('화면에 바로 보일런지는 모르겠지만, 자네의 체력은 이제 %d이네', me:maxhp()), true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_5
        end

::ROUTINE_BUY_HP_7::
        button = npc:dialog(me, '강인한 체력을 나쁜 곳에 쓰려고 한다면... 내가 자네를 가만두지 않겠네. 알겠나?', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_6
        else
            goto ROOT
        end

    elseif index == 5 then
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

::ROUTINE_CHANGE_FACE_1::
        button = npc:dialog(me, '지금부터 차례로 자네가 갖을 수 있는 모습을 보여주도록 하겠다. 수술이 그리 쉽지는 않고', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = npc:dialog(me, '원하는 모습이 보이면 바로 예를 선택하도록. 현재 네가 가질 수 있는 얼굴은 101개가 있다. 이전이나 다음을 선택해도 얼굴이 바뀌지 않으면 마지막 모습이니 그리 알도록.', true, true)
        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_CHANGE_FACE_1
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
                    goto ROOT
                end

                me:look(face)
                me:money(money - 1000000)
                if npc:dialog(me, '새 얼굴이 마음에 드는가? 맘에 안들어도 다시 해 줄 수는 없네.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                if npc:dialog(me, '나에게 더 도움을 받을 일은 이제 없겠지?.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                goto ROOT
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