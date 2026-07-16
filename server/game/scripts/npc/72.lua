-- npc: 폭12지입장
function NPC_72(me, npc)
    local selected, button = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'PK12지입장', '신부여성입장'})
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if selected == 1 then
        ::NPC_72_0001::
        button = me:dialog(npc, '그래, 여길 들어가고 싶단 말이지?', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_72_0002::
        button = me:dialog(npc, '이 안은 아주 위험하다네...', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0001
        end

        ::NPC_72_0003::
        button = me:dialog(npc, '폭탄으로 죽을 수도 있고, 사용자들끼리도 서로 죽일 수 있다네...', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0002
        end

        ::NPC_72_0004::
        button = me:dialog(npc, '이 안에서 죽어서 아이템이나 경험치를 잃어버린 것은 자네가 책임져야 하네...', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0003
        end

        selected, button = me:list(npc, '아주 위험한 곳인데, 그래도 들어가길 원하는가?', {'네. 들어가길 원합니다.', '아니오. 다시 생각하니 겁나는군요.'}, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0004
        end

        if selected == 2 then
            me:dialog(npc, '잘 생각 했네. 잘가게...', false, false)
            return
        end

        ::NPC_72_0005::
        button = me:dialog(npc, '각오가 대단한가보군... 마지막으로 묻겟네...', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_72_0006::
        button = me:dialog(npc, '이 안에서 서로 죽이거나 죽어서 생긴 모든 문제는 자네 책임일세...', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0005
        end

        ::NPC_72_0007::
        button = me:dialog(npc, '죽어서 아이템이나 경험치를 잃은 것도 모두 자네 책임이고 복구는 안된다네...', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0006
        end

        selected, button = me:list(npc, '여기에 동의하는가?', {'네. 모두 동의합니다.', '아니오. 다시 생각하니 겁나는군요.'}, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0007
        end

        if selected == 2 then
            return
        end

        button = me:dialog(npc, '그럼, 행운을 비네... 조심하게..', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local map = name2map('PK12지신의유적')
        if map == nil then
            me:dialog(npc, '존재하지 않는 맵입니다.', false, true)
            return
        end
        local x = 25 + math.random(1, 10)
        local y = 12 + math.random(1, 5)
        me:map(map, {x, y})

    elseif selected == 2 then
        ::NPC_72_0010::
        button = me:dialog(npc, '그래, 무슨일이지?', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_72_0011::
        button = me:dialog(npc, '신부여성으로 가고 싶다고?', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0010
        end

        ::NPC_72_0012::
        button = me:dialog(npc, '신부여성은 참 좋은곳이지..', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0011
        end

        ::NPC_72_0013::
        button = me:dialog(npc, '신부여성에서는 많은 곳을 여행할 수 있다네..', true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0012
        end

        selected, button = me:list(npc, '정말 신부여성으로 가겠는가?', {'네. 보내주세요.', '아니오. 가기싫어요.'}, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_72_0013
        end

        if selected == 2 then
            me:dialog(npc, '잘 생각 했네. 잘가게...', false, false)
            return
        end

        button = me:dialog(npc, '그럼, 행운을 비네...', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local map_names = {'신부여서쪽주막', '신부여서쪽주막2'}
        local map_name = map_names[math.random(1, 2)]
        local map = name2map(map_name)
        if map == nil then
            me:dialog(npc, '존재하지 않는 맵입니다.', false, true)
            return
        end
        me:map(map)
    end
end