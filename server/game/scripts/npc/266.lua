-- npc: 길림동굴보초

return {
    on_click = function(me, npc)
        if me:dialog(npc, '멈춰라!\n이곳은 이제 아무나 들이지 말라는 어명이 있었네.\n어떤 몰상식한 자가 이곳에서 한몫 잡아 보려고 다른 사람들에게 피해를 줬다는데, 요즘 사람들은 왜 이렇게 자기 생각만 하는지 모르겠네.. 에휴~\n그나저나 무엇을 하려던 참인가?', { prev = false, next = true }) == DIALOG_RESULT.QUIT then
            return
        end

        local sel = me:list(npc, '무엇을 하시겠습니까?', {
            '길림동굴에 입장하려고요.',
            '수렴귀방에 입장하려고요.',
        })
        if sel == nil then
            return
        end

        local warp_direct = false
        if sel == 2 then
            local selected, button = me:list(npc, '바로 수렴귀방으로 입장하려면 참나무껍질을 바쳐야하오 입장하겠소?', { '네. 입장할래요.', '아니요. 아깝네요.' })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if selected ~= 1 then
                return
            end

            warp_direct = true
            return
        end

        local selected, button = me:list(npc, '그대를 포함한 일행(최대 10명)의 동시입장이 가능하다\n단, 그대의 시야에서 벗어나 있는 사람은 입장이 안된다는 사실을 명심하거라.\n첫번째 입장 후 20시간 동안 3회만 입장이 가능하다는 첨을 명심하거라.\n무엇을 원하는가?', {
            '지금 바로 입장하겠습니다.',
            '제 일행 중 입장 불가능한 사람이 있는지요?',
            '나중에 다시..',
        })

        if button == DIALOG_RESULT.QUIT then
            return
        end

        if selected ~= 1 then
            local map = nil
            local position = {x = 0, y = 0}
            if warp_direct then
                map = name2map('수렴귀방')
                position = {x = 10, y = 9}
            else
                map = name2map('길림동굴')
                position = {x = 10, y = 9}
            end
            me:map(map, position)
        end

        if selected == 2 then
            return
        end
    end
}
