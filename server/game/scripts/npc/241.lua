-- npc: 주경원
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_241(me, npc)
    local q = me:quest(quest.QUEST_ALCOHOLIC_DRINK)
    local btn

    if q == nil then
        ::NPC_241_0001::
        btn = me:dialog(npc, '안녕하시오. ' .. me:name() .. '씨, 처음 뵙겠소. 난 여기 남양에서 뱀술을 만드는 주경원이라고 하오.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_241_0002::
        btn = me:dialog(npc, '나는 20년동안 뱀술을 만들면서 살아왔지. 우리 집안 대대로 전해지는 특별한 방법으로 만드는데, 많은 재료가 필요하지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0001
        end
        ::NPC_241_0003::
        btn = me:dialog(npc, '그런데 요즘은 남양에도 괴물들이 많아서 땅꾼들이 뱀을 잡을 수가 없단 말이네. 뱀술을 만드는 게 내 유일한 낙인데...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0002
        end
        local sel, lb = me:list(npc, '뱀이 필요한데.. 어떻게 구한담?', { '제가 도와드리죠. 뱀을 구해 드리면 되는 건가요?', '뱀술말고 다른 술을 만드시면 되지 않나요?' }, false)
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel == 1 then
            if sel == 1 then
                me:dialog(npc, '그건 용납할 수 없네! 나는 오로지 뱀술만을 만들 것이네!', false, false)
            end
            return
        end
        if sel ~= 0 then
            return
        end
        q = me:start_quest(quest.QUEST_ALCOHOLIC_DRINK)
        if q == nil then
            return
        end
        q:step(1)
        me:push_achievement(40, '청심사주의 재료를 구하자.[1/2]', 7, 20)
        me:dialog(npc, '아아! 고맙네 고마워. 재료를 구해주면 내가 만든 술을 한병 주겠네.\n\n내가 만들려는 것은 그냥 뱀술이 아니고, \'청심사주\'라는 것이지. 보통 뱀술도 몸에 좋다고 하지만, 청심사주에 비할바는 아니야. 뱀고기 100개와 좋은뱀고기 5개를 농축하여 만든다네.', false, true)
        return
    end

    if q:step() == 0 then
        ::NPC_241_0010::
        btn = me:dialog(npc, '안녕하시오. ' .. me:name() .. '씨, 처음 뵙겠소. 난 여기 남양에서 뱀술을 만드는 주경원이라고 하오.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_241_0011::
        btn = me:dialog(npc, '나는 20년동안 뱀술을 만들면서 살아왔지. 우리 집안 대대로 전해지는 특별한 방법으로 만드는데, 많은 재료가 필요하지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0010
        end
        btn = me:dialog(npc, '그런데 요즘은 남양에도 괴물들이 많아서 땅꾼들이 뱀을 잡을 수가 없단 말이네. 뱀술을 만드는 게 내 유일한 낙인데...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0011
        end
        local sel, lb = me:list(npc, '뱀이 필요한데.. 어떻게 구한담?', { '제가 도와드리죠. 뱀을 구해 드리면 되는 건가요?', '뱀술말고 다른 술을 만드시면 되지 않나요?' }, false)
        if lb == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel == 1 then
            me:dialog(npc, '그건 용납할 수 없네! 나는 오로지 뱀술만을 만들 것이네!', false, false)
            return
        end
        if sel ~= 0 then
            return
        end
        q:step(1)
        me:push_achievement(40, '청심사주의 재료를 구하자.[1/2]', 7, 20)
        me:dialog(npc, '아아! 고맙네 고마워. 재료를 구해주면 내가 만든 술을 한병 주겠네.\n\n내가 만들려는 것은 그냥 뱀술이 아니고, \'청심사주\'라는 것이지. 보통 뱀술도 몸에 좋다고 하지만, 청심사주에 비할바는 아니야. 뱀고기 100개와 좋은뱀고기 5개를 농축하여 만든다네.', false, true)
        return
    end

    if q:step() == 1 then
        if not me:has_items({['뱀고기'] = 100, ['좋은뱀고기'] = 5}) then
            me:dialog(npc, '청심사주를 만들기 위해선 뱀고기 100개와 좋은뱀고기 5개가 필요하네.', false, false)
            return
        end
        ::NPC_241_0020::
        btn = me:dialog(npc, '오오, 뱀고기 100개와 좋은뱀고기 5개를 정말 구해왔군. 고맙네 고마워.. 이제 뱀술을 한 번 만들어 보겠네.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_241_0021::
        btn = me:dialog(npc, '........\n\n아아 이럴수가... 오랫동안 뱀술을 만들지 못해 얼마간 재료 살피는 것을 소홀히 했더니 재료가 몽땅 못쓰게 되었군. 청심사주엔 뱀만 필요한 것이 아니거든.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0020
        end
        local sel = me:list(npc, '이 일을 어쩐다....', { '내친김에 나머지 재료도 제가 구해다 드리지요.', '에이, 이젠 저도 몰라요.' }, false)
        if sel == nil or sel ~= 0 then
            return
        end
        q:step(2)
        me:push_achievement(40, '청심사주의 재료를 구하자.[2/2]', 7, 20)
        me:mkspell('청심초')
        me:rmitem({['뱀고기'] = 100, ['좋은뱀고기'] = 5}, ITEM_DELETE_TYPE.GIVE)
        ::NPC_241_0030::
        btn = me:dialog(npc, '아아 고맙네, 그럼 일단 뱀고기는 내가 가져가겠네..\n\n나머지 재료는 기린의피와 청심초 5뿌리라네. 기린의피는 붉은색의 기린을 잡아서 피를 받아오면 되네.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_241_0031::
        btn = me:dialog(npc, '청심초는 남경지역을 한참 돌아다니다 보면 운이 좋은 사람에게 발견된다네.\n\n기린의피와 청심초 5뿌리만 있으면 이제 정말 만들 수 있을 것이네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_241_0030
        end
        return
    end

    if q:step() == 2 then
        local materials = {['기린의피'] = 1, ['청심초'] = 5}
        if not me:has_items(materials) then
            me:dialog(npc, '재료가 모자라군.. 기린의피와 청심초 5뿌리만 있다면 이젠 정말 만들수 있을 것이네.', false, false)
            return
        end
        ::NPC_241_0040::
        btn = me:dialog(npc, '오오, 고맙네 고마워.. 내 뱀술을 만들어 줄 테니 잠시만 기다리게!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '... ... ...\n\n다 됐네! 한 병은 평소에 신세가 많았던 천선도사에게 좀 가져다 드리고, 한병은 자네가 마시게. 허허허.. 정말 고마우이..', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['청심사주'] = 2 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '재료가 모자라군.. 기린의피와 청심초 5뿌리만 있다면 이젠 정말 만들수 있을 것이네.', false, false)
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with('청심사주', '을', '를') .. ' 줄 수 없네.', false, true)
            return
        end
        q:step(3)
        me:push_achievement(40, '청심사주를 천선도사에게 전해주자.', 7, 20)
        return
    end

    me:dialog(npc, '어떤가? 청심사주의 맛이 아주 기가 막히지 않은가?', true, true)
end
