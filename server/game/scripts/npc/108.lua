-- npc: 도성연
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_DOJAEYOUNG_HERB)
        if q == nil then
            me:dialog(npc, '쿨럭..쿨럭..', { prev = false, next = true })
            return
        end

        if q:step() < 2 then
            ::NPC_108_0001::
            local btn = me:dialog(npc, '쿨럭..쿨럭..', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0002::
            btn = me:dialog(npc, '(안색이 별로 좋지 않다.)', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0001
            end
            return
        end

        if q:step() == 2 then
            ::NPC_108_0010::
            local btn = me:dialog(npc, '오.. ' .. me:name() .. '씨 아닌가. 아들놈에게 약초를 구해셨다고 하더군요. 덕분에 내가 이렇게 건강을 되찾았소. 고맙구려.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0011::
            btn = me:dialog(npc, '그나저나 말이지, 내 아들놈이 약초를 구해준게 기특해서 내가 선물을 하나 주려고 하는데 말이야, 아아 물론 구하기는 ' .. me:name() .. '씨가 구했지만서도, 아들놈도 고생을 했다고.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0010
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0012::
            btn = me:dialog(npc, '예전에 도삭산을 지나가는데 인어들이 방울을 가지고 있는게 보이던데, 인어의방울이라고 하면 집에 걸어두기만 해도 흉한 것들이 집에 접근을 못한다고 하거든', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0011
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0013::
            btn = me:dialog(npc, '내가 건강이 나빠진것도 도삭산의 흉한 기운 때문인듯 한데, 아들놈한테 방울 하나쯤 주고 싶구만.\n\n내 직접 가진 못하고, 어떻게든 구했으면 좋겠는데...', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0012
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel = me:list(npc, me:name() .. '씨가 하나 구해주시지 않겠소?', { '제가 구해드리겠습니다.', '저는 좀 바빠서..' }, { prev = false })
            if sel == nil then
                return
            end
            if sel == 2 then
                me:dialog(npc, '허허.. 바쁘시다면 할 수 없군요.', { prev = false, next = true })
                return
            end
            if sel ~= 1 then
                return
            end
            q:step(3)
            me:push_achievement(9, '도성연의 부탁을 들어주자!', 7, 1)
            ::NPC_108_0015::
            btn = me:dialog(npc, '오오. 고맙소. 인어의방울..은 도삭산고양이인어가 가지고 있지 않을까? 고양이목에 방울달기라는 말도 있지 않소? 하하하. 농담이지만 말이지.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0016::
            btn = me:dialog(npc, '뫼롱씨는 내 생명의 은인이니 292층으로 가는 지름길로 보내주지.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0015
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local warp_sel, warp_btn = me:list(npc, '지금 지름길로 292층으로 가시겠소?', { '네, 보내주십시오.', '나중에 가겠습니다.' })
            if warp_btn ~= DIALOG_RESULT.QUIT and warp_sel == 1 then
                local map = name2map('도삭산292층')
                if map ~= nil then
                    me:map(map, {16, 37})
                end
            end
            return
        end

        if q:step() == 3 then
            local btn
            if not me:has_items('인어의방울', 1) then
                local warp_sel, warp_btn = me:list(npc, '아직 인어의방울을 구하지 못하셨군. 지금 지름길로 292층으로 가시겠소?', { '네, 보내주십시오.', '나중에 가겠습니다.' })
                if warp_btn ~= DIALOG_RESULT.QUIT and warp_sel == 1 then
                    local map = name2map('도삭산292층')
                    if map ~= nil then
                        me:map(map, {16, 37})
                    end
                end
                return
            end
            ::NPC_108_0020::
            btn = me:dialog(npc, '오.. 이것이 인어의방울이군. 고맙소고맙소.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0021::
            btn = me:dialog(npc, '그런데 이거 정말 미안한 얘긴데.. 내 딸이 이 얘길 듣더니, 왜 오빠한테만 선물을 주냐고 칭얼대는거야. 자기도 인어의거울을 가지고 싶다나?', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0020
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0022::
            btn = me:dialog(npc, '인어의거울은 보면볼수록 얼굴이 예뻐진다는 얘기가 있는데, 그걸 어디서 주워들었는지 원참...', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0021
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel = me:list(npc, me:name() .. '씨, 인어의거울도 좀 구해다 주실수 없겠소?', { '네, 구하는 김에 거울도 구하지요.', '갑자기 바쁜 일이 생겨서...' }, { prev = false })
            if sel == nil then
                return
            end
            if sel == 2 then
                me:dialog(npc, '그런가? 흐음..딸이 하도 칭얼대서 말이지..', { prev = false, next = true })
                return
            end
            if sel ~= 1 then
                return
            end
            q:step(4)
            me:rmitem('인어의방울', 1, ITEM_DELETE_TYPE.GIVE)
            btn = me:dialog(npc, '정말 고맙소. 인어의거울은.. 도삭산이쁘니인어가 가지고 있으려나? 하하하', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local warp_sel, warp_btn = me:list(npc, '지금 지름길로 292층에 가시겠소?', { '네, 보내주십시오.', '나중에 가겠습니다.' })
            if warp_btn ~= DIALOG_RESULT.QUIT and warp_sel == 1 then
                local map = name2map('도삭산292층')
                if map ~= nil then
                    me:map(map, {16, 37})
                end
            end
            return
        end

        if q:step() == 4 then
            local btn
            if not me:has_items('인어의거울', 1) then
                local warp_sel, warp_btn = me:list(npc, '아직 인어의거울을 구하지 못하셨군. 지금 지름길로 292층으로 가시겠소?', { '네, 보내주십시오.', '나중에 가겠습니다.' })
                if warp_btn ~= DIALOG_RESULT.QUIT and warp_sel == 1 then
                    local map = name2map('도삭산292층')
                    if map ~= nil then
                        me:map(map, {16, 37})
                    end
                end
                return
            end
            ::NPC_108_0030::
            btn = me:dialog(npc, '오,, 이것이 인어의거울이군! 고맙소!! 정말 고맙소!', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_108_0031::
            btn = me:dialog(npc, '이걸 어떻게 사례하나.. 그렇지. 우리 집안에 내려오는 목걸이를 하나 드리지.\n\n\'봉마의목걸이\'라고 해서.. 마귀를 봉인하여 담았다고 하는 옥으로 만든것이란느데.. 정말인지는 모르겠소.', { prev = false, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_108_0030
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = { ['인어의거울'] = 1 } },
                { ['item'] = { ['봉마의목걸이'] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '인어의거울을 가지고 있지 않으시군요.', { prev = false, next = true })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 봉마의목걸이를 받을 수 없습니다.', { prev = false, next = true })
                return
            end
            q:step(5)
            me:push_achievement(9, '도삭산 200층 퀘스트 완료', 7, 1)
            me:dialog(npc, '뭐 어쨌든 기념으로 드릴테니.. 잘 쓰시오..\n\n그럼 건강하시오!', { prev = false, next = true })
            return
        end

        me:dialog(npc, me:name() .. '씨군, 요세 잘 지내나? 당신 덕분에 감사한일이 정말 많소.', { prev = true, next = true })
    end
}
