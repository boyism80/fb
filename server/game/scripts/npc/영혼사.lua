function NPC_28(me, npc)

::ROUTINE_INIT_1::
    local index = nil
    local button = me:dialog(npc, '나에게 또 무슨 도움을 받겠다고 찾아왔느냐...', false, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end

::ROUTINE_INIT_2::
    button = me:dialog(npc, '여기 저기서 사고나 치고 돌아다니면서, 툭하면 나를 찾아오는 속셈이 도대체 무엇이냐?', true, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end

    if button == DIALOG_RESULT_PREV then
        goto ROUTINE_INIT_1
    end

    button = me:dialog(npc, '그리고 나를 찾아올 생각을 했다면 뭔가를 들고 왔어야지... 만약 뭔가를 공짜로 얻으려고 생각했다면 큰 오산이니 바로 돌아가거라.', true, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end

    if button == DIALOG_RESULT_PREV then
        goto ROUTINE_INIT_2
    end

::ROOT::
    index, button = me:list(npc, '그래도 버티고 서서 도대체 원하는 것이 뭐냐?', {'힘올리기', '지력올리기', '민첩올리기', '체력사기', '마력사기', '성형', '성전환'}, false)
    if index == 3 then
::ROUTINE_BUY_HP_1::
        button = me:dialog(npc, '체력을 사기 전에 명심하게.. 지금 착용하고 있는 모든 아이템을 벗은 후, 체력을 사기 바라네.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

::ROUTINE_BUY_HP_2::
        button = me:dialog(npc, '만약, 내 말을 듣지 않고, 체력을 사서 일어나는 불이익에 대해서는 절대 책임을 질 수 없네!!', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_1
        end

::ROUTINE_BUY_HP_3::
        button = me:dialog(npc, '경험치로 체력을 사고 싶다고.... 늘 이렇게 경험치를 팔고는 여기 저기 가서 잘못 팔았다고 후회를 할 것이거늘... 쯧쯧... 어디 레벨이나 되는지 한 번 보자.', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_2
        end

        button = me:dialog(npc, '레벨은 충분하고...', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_3
        end

        local step = me:input(npc, '그래 체력을 몇 단계나 올리려고? 체력은 한 단계가 50이라는 것은 알고 있겠지?', '예,', '단계를 올리고 싶습니다.')
        local exp = step * 10000000
        local my_exp = me:exp()
        if my_exp < exp then
            me:dialog(npc,  '경험치가 모자람')
            goto ROOT
        end

        me:exp(my_exp - exp)
        me:base_hp(me:base_hp() + (50 * step))

::ROUTINE_BUY_HP_5::
        button = me:dialog(npc, string.format('자네의 경험치로 체력을 %d단계 올렸으니 그리 알고 돌아가도록 해.', step), false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

::ROUTINE_BUY_HP_6::
        button = me:dialog(npc, string.format('화면에 바로 보일런지는 모르겠지만, 자네의 체력은 이제 %d이네', me:maxhp()), true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_5
        end

::ROUTINE_BUY_HP_7::
        button = me:dialog(npc, '강인한 체력을 나쁜 곳에 쓰려고 한다면... 내가 자네를 가만두지 않겠네. 알겠나?', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_BUY_HP_6
        else
            goto ROOT
        end

    elseif index == 5 then
        index, button = me:list(npc, '지금의 모습을 벗어나 새로운 얼굴을 갖기를 원하고 있나?', {'예', '아니오'})
        if index ~= 0 then
            return
        end

        index, button = me:list(npc, '사회에서 물의를 빚고 숨어서 지내려는 것은 아니겠지?', {'아닙니다.', '예. 잠시 숨어서....'})
        if index ~= 0 then
            return
        end

        index, button = me:list(npc, '수술을 하려면 말일세, 백만전의 금이 필요하다네.. 가지고 있는가?', {'예', '아니오'})
        if index ~= 0 then
            return
        end

::ROUTINE_CHANGE_FACE_1::
        button = me:dialog(npc, '지금부터 차례로 자네가 갖을 수 있는 모습을 보여주도록 하겠다. 수술이 그리 쉽지는 않고', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = me:dialog(npc, '원하는 모습이 보이면 바로 예를 선택하도록. 현재 네가 가질 수 있는 얼굴은 101개가 있다. 이전이나 다음을 선택해도 얼굴이 바뀌지 않으면 마지막 모습이니 그리 알도록.', true, true)
        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_CHANGE_FACE_1
        end

        if button == DIALOG_RESULT_QUIT then
            return
        end

        local hair = 0
        while true do
            local portrait = {
                ['hair'] = hair
            }
            index, button = me:list(portrait, '다음에 또 수술을 해 줄 수는 있지만, 수술을 계속한다고 결코 좋아지지는 않는다. 이 모습을 선택하겠느냐?', {'선택', '이전 모습으로', '다음 모습으로', '수술 포기'}, false)
            if index == nil then
                return
            end

            if index == 0 then
                if me:dialog(npc, '그럼 네 얼굴을 그 모습을 고쳐주도록 하지.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                local money = me:money()
                if money < 1000000 then
                    if me:dialog(npc, '백만원이 없잖아 임마', false, true) == DIALOG_RESULT_QUIT then
                        return
                    end
                    goto ROOT
                end

                me:look(hair)
                me:money(money - 1000000)
                if me:dialog(npc, '새 얼굴이 마음에 드는가? 맘에 안들어도 다시 해 줄 수는 없네.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                if me:dialog(npc, '나에게 더 도움을 받을 일은 이제 없겠지?.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                goto ROOT
            elseif index == 1 then
                hair = (hair - 1 + 102) % 102
            elseif index == 2 then
                hair = (hair + 1) % 102
            elseif index == 3 then
                me:dialog(npc, '수술 포기')
                return
            end
        end
    elseif index == 6 then
::ROUTINE_CHANGE_SEX_1::
        button = me:dialog(npc, '이건 아직 해 줄 수가 없네...', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = me:dialog(npc, '나도 조금 더 공부를 해야 하고... 그리고 하여간 여러가지 복잡한 사정이 있으니... 이번ㅂ에는 그냥 돌아가도록 하게.', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_CHANGE_SEX_1
        end

        index, button = me:list(npc, '아니 돌아가라고 하는데, 왜 이렇게 나를 귀찮게 하지? 이것을 꼭 해야 할 사정이 있나? 비용도 만만치 않게 들고, 그리고 실패할 확률도 많은데, 그래도 꼭 해야 되겠나?', {'예', '아니오. 그럼 나중에...'})
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if index ~= 0 then
            return
        end

        button = me:dialog(npc, '정 그렇다면... 먼저 아이템을 장비하고 있는지 보겠네. 아이템을 장비한 채로 성전환을 하면 부작용이 있을 수도 있지.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        local equipments = me:equipments()
        local naked = true
        for parts, equipment in pairs(equipments) do
            naked = false
            break
        end
        if not naked then
            me:dialog(npc, '장비를 착용중이라서 불가능')
            return
        end

        button = me:dialog(npc, '아이템은 장비하지 않았고... 필요한 수술비를 가지고 왔는지 좀 보겠네.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end
        local money = me:money()
        local price = 100000
        if price > money then
            me:dialog(npc, '돈이 모자람')
            return
        end

        button = me:dialog(npc, '성을 전환하려면 십만전이 필요하다네... 금전을 지불하면 돌려주지 않을 것이니 지금 신중하게 생각하시게. 왜냐하면... 하여간 그렇다네. 그 정도는 준비해 왔겠지?', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = me:dialog(npc, '먼저 수술비를 받았으니... 이제 자네를 조금 검사해 보아야 되겠네.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        button = me:dialog(npc, '음... 아직 결혼한 상태가 아니니... 수술을 시작해 봐야지.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        local sex = me:sex()
        local sex_from = nil
        local sex_to = nil
        if sex == SEX_MAN then
            sex_from = '남자'
            sex_to = '여자'
        else
            sex_from = '여자'
            sex_to = '남자'
        end
        index, button = me:list(npc, string.format('자네 %s가 되고 싶다는 것이지?', sex_to), {'예', '아니오'})
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if index ~= 0 then
            return
        end

        button = me:dialog(npc, '그래. 내 수술비도 받고, 자네가 결혼한 몸도 아니라는 것을 확인했으니, 성별을 바꾸어는 주겠네. 하지만 이런 일을 다시는 내게 와서 부탁하지 말게. 알겠나?', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        me:money(money - price)
        if sex == SEX_MAN then
            me:sex(SEX_WOMAN)
        else
            me:sex(SEX_MAN)
        end
        button = me:dialog(npc, string.format('자네 지금부터는 %s 되었네. %s용 의복을 갖추고 있는지는 몰라도, 하여간 자네는 이제 %s의 몸을 갖게 되었으니 그렇게 알고 돌아고도록 하게.', sex_to, sex_from, sex_to), false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end
        goto ROOT
    end
end