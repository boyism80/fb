-- npc: 천선도사
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_186(me, npc)
    local q = me:quest(quest.QUEST_ALCOHOLIC_DRINK)
    local btn

    if q ~= nil and q:step() == 3 then
        if me:has_items('청심사주', 1) then
            ::NPC_186_0001::
            btn = me:dialog(npc, '허허, 주경원이 청심사주를 만드는것을 자네가 도와주었지? 내 신통력으로 다 보고 있었지.', true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_186_0002::
            btn = me:dialog(npc, '정말 오랫만에 보는 술이군! 고맙네. 이 술을 다 마셔버리지 않고 가져다 준 정직한 자네에게 선물을 주고 싶군. 요즘은 자네같은 사람도 드물다네. 내 고대금속조각\'음을 하나 주지. 어딘가에 쓸모가 있을게야..', false, false)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_186_0001
            end
            local code = me:exchange(
                { ['item'] = { ['청심사주'] = 1 } },
                { ['item'] = { ["고대금속조각'음"] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '청심사주를 가지고 있지 않으시군요.', false, true)
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 ' .. name_with("고대금속조각'음", '을', '를') .. ' 줄 수 없네.', false, true)
                return
            end
            q:step(4)
            me:push_achievement(40, '청심사주를 천선도사에게 전해주었다.', 7, 20)
            return
        end
    end

    me:dialog(npc, '압록강의 물은 정말 신비롭지...', true, true)
end
