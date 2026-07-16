-- npc: 돈유합의달인
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_184(me, npc)
    local q = me:quest(quest.QUEST_JOWANG)
    local btn

    if q == nil then
        me:dialog(npc, '안녕하시오. 장안성에는 무슨일로 오셨는가?', { prev = false, next = true })
        return
    end

    if q:completed() then
        me:dialog(npc, '흐음..', { prev = false, next = true })
        return
    end

    if q:step() == 0 then
        local sel = me:list(npc, '자네는 왜 날 찾아왔나? 무슨 볼일이라도?', { '화기삼동충초돈유합의 달인이라 하여 찾아왔습니다.', '아닙니다...지나가는 길입니다.' })
        if sel == nil then
            return
        end
        if sel ~= 1 then
            me:dialog(npc, '허허. 그러시오?', { prev = false, next = true })
            return
        end
        ::NPC_184_0001::
        local btn = me:dialog(npc, '아하...그런것이라면 내가 도와줄 수 있지. 나도 공짜로는 도와줄수 없으니 내가 필요한 물건좀 구해다주게. 약재로 사용할 인삼과 동충하초가 급히 필요하거든. 부탁하네.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_184_0010::
        btn = me:dialog(npc, '동충하초는 대방성입구에, 인삼은 국경지대를 뒤져보면 나올걸세.', { prev = false, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_184_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q:step(1)
        me:push_achievement(31, '돈유합의달인의 부탁을 들어주자. [1/2]', 7, 1)
        return
    end

    if q:step() == 1 then
        local materials = {['동충하초'] = 1, ['인삼'] = 1}
        if not me:has_items(materials) then
            me:dialog(npc, '아직 재료가 부족한것 같소. 동충하초는 대방성입구, 인삼은 국경지대를 뒤져보면 나올걸세.', { prev = false, next = true })
            return
        end
        ::NPC_184_0020::
        btn = me:dialog(npc, '허허 고맙구만. 이 재료들은 내가 잘 사용하도록 하지.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_184_0021::
        btn = me:dialog(npc, '그럼 화기삼동충초돈유합의 재료를 알려주겠네. 비둘기고기,인삼,동충하초,구기자 이렇게 네가지의 재료가 필요하다네. 구해오면 내가 직접 요리해줄테니 재료를 찾아오게.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_184_0020
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_184_0022::
        btn = me:dialog(npc, '동충하초와 인삼은 어디서 구할 수 있는지 알고 있지? 비둘기고기와 구기자는 귀하기 때문에 아무곳에서나 팔지는 않고 중국 어딘가의 푸줏간에서 살 수 있을걸세.', { prev = false, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_184_0021
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:rmitem(materials, ITEM_DELETE_TYPE.GIVE)
        q:step(2)
        me:push_achievement(31, '돈유합의달인의 부탁을 들어주자. [2/2]', 7, 1)
        return
    end

    if q:step() == 2 then
        local materials = {['동충하초'] = 1, ['인삼'] = 1, ['비둘기고기'] = 1, ['구기자'] = 1}
        if not me:has_items(materials) then
            me:dialog(npc, '아직 재료를 다 구하지 못했군. 재료는 동충하초,인삼,비둘기고기,구기자를 구해오게.', { prev = false, next = true })
            return
        end
        ::NPC_184_0030::
        btn = me:dialog(npc, '재료를 다 구해왔군. 내가 직접 요리해주겠네.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_184_0031::
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['화기삼동충초돈유합'] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 재료를 다 구하지 못하셨군요.', { prev = false, next = true })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 화기삼동충초돈유합을 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        q:step(3)
        btn = me:dialog(npc, '자~ 다됐네. 귀환 음식이니 좋은 곳에 쓰도록 하게.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:push_achievement(31, '돈유합의달인의 부탁을 들어주었다.', 7, 1)
        return
    end
end