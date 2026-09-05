-- npc: 장돌뱅이
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_RABBIT_CATCH = 20
local ITEM_NEURAEJINDO = '뇌진도'

local function run_cidequest_jangdol(me, npc)
    local q = me:quest(quest.QUEST_CIDEQUEST)
    if q == nil or q:step() ~= 1 then
        me:dialog(npc, "지금은 대화할 때가 아닌 것 같군.", { prev = false, next = false })
        return
    end
    local pn = (q and q:progress()) or 0
    if me:has_items(ITEM_NEURAEJINDO, 1) then
        me:dialog(npc, "오. 뇌진도를 가지고 왔구만.", { prev = false, next = true })
        if not me:rmitem(ITEM_NEURAEJINDO, 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        me:dialog(npc, "사실 그 보물은 방천화극을 팔고 있던 내 친구가 들고 가는 것을 보았네.\n\n그러니 그 친구에게 찾아가 보시게나.", { prev = false, next = false })
        q:progress(2)
        return
    end
    if pn == 2 then
        me:dialog(npc, "그 친구는 부여미궁 어딘가에 있다고 하더군...", { prev = false, next = false })
        return
    end
    local list_1, b1 = me:list(npc, "음...", { "혹시 용왕님의 보물에 대해 알고 계신가요?" }, { prev = false })
    if b1 == DIALOG_RESULT.QUIT then
        return
    end
    if list_1 ~= 1 then
        return
    end
    local list_2, b2 = me:list(npc, "물론 알고 있지.", { "그렇다면 가르쳐 주실 수 있으신지요?" }, { prev = false })
    if b2 == DIALOG_RESULT.QUIT or list_2 ~= 1 then
        return
    end
    local list_3, b3 = me:list(npc, "이 사람아. 세상에 공짜가 어디있는가?.", { "..." }, { prev = false })
    if b3 == DIALOG_RESULT.QUIT or list_3 ~= 1 then
        return
    end
    local list_4, b4 = me:list(npc, "뇌진도를 가지고 온다면 내 어디 있는지 말해주지.", { "좋습니다.", "뇌진도를 가지고 오겠습니다." }, { prev = false })
    if b4 == DIALOG_RESULT.QUIT then
        return
    end
    me:dialog(npc, "기다리고 있겠네.", { prev = false, next = false })
    q:progress(1)
end

return {
    on_click = function(me, npc)
        local main_sel, main_btn = me:pursuit(npc, "안녕하세요. 어떻게 오셨나요?", {
            '물건 사기',
            '물건 팔기',
            '별주부전',
            '용왕의보물',
            '금은보화',
        })
        if main_btn == DIALOG_RESULT.QUIT then
            return
        end
        if main_sel == 1 or main_sel == 2 then
            me:dialog(npc, "준비중입니다.", { prev = false, next = false })
            return
        end
        if main_sel == 4 then
            run_cidequest_jangdol(me, npc)
            return
        end
        if main_sel == 5 then
            local code = me:exchange(
                { ['item'] = { ["금은보화"] = 1 } },
                { ['money'] = 15000 }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "금은보화를 가지고 있거든 말을 걸어주게나...", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "금전을 받을 여유가 없군요.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "이 귀한것을 가지고 있다니... 자, 여기 만오천전을 줄테니 어서 가져가게나.", { prev = false, next = false })
            return
        end
        local q = me:quest(quest.QUEST_RABBIT_LIVER)
        if q == nil then
            me:dialog(npc, '지금은 이야기를 할 때가 아니군..', { prev = false, next = false })
            return
        end

        local sel = me:list(npc, '어쩐일이신가?', { '토끼의간을 구해야되는데..', '뇌진도를 드리겠습니다.' }, { prev = false })
        if sel == nil then
            return
        end

        if sel == 1 then
            ::NPC_159_0001::
            local btn = me:dialog(npc, '음.. 거북장군에게서 토끼의간을 구해오라는 부탁을 받았단 말아지. 그런데 그 토끼는 정말 희귀한 토끼라네. 보통토끼와는 다르지', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_159_0002::
            btn = me:dialog(npc, '나도 장사 하느라 전국 방방곡곡 안가보는데가 없지만, 그 토끼는 딱 한 번 밖에 못봤었지.. 어디서 보았냐 하면 말일세...', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_159_0001
            end
            ::NPC_159_0003::
            btn = me:dialog(npc, '흐흐흐.. 이보게, 하지만 말일세.. 조금 야속하게 들릴지 모르겠지만, 나도 장사꾼이 아닌가?', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_159_0002
            end
            ::NPC_159_0004::
            local choice, list_btn = me:list(npc, '흐흐흐...', { '원하는게 뭡니까' }, { prev = false })
            if choice == nil or choice ~= 1 then
                return
            end
            me:dialog(npc, '요즘 한창 장사가 되는 물건이 뇌진도라네.. 그걸 하나 구해오면 내 분명히 자네에게 그 토끼가 출현하는 장소를 가르쳐 줌세.', { prev = false, next = false })
            return
        end
        if not me:has_items(ITEM_NEURAEJINDO, 1) then
            me:dialog(npc, '뇌진도가 없지 않은가?', { prev = false, next = false })
            return
        end
        q:step(2)
        me:rmitem(ITEM_NEURAEJINDO, 1, ITEM_DELETE_TYPE.GIVE)
        me:push_achievement(ACHIEVEMENT_RABBIT_CATCH, '토깽이를 생포하자.', 7, 1)
        ::NPC_159_0005::
        btn = me:dialog(npc, '그 토끼가 출현하는 곳은 다름 아닌 한두고개라네.. 나도 한두고개에서 깊숙히 안들어가봐서 깊숙한 곳에서도 출현할지 어떨지는 모르겠네만', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_159_0006::
        btn = me:dialog(npc, '분명한 것은 한두고개에서 깊지 않은 곳에서 봤다는 사실일세.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_159_0005
        end
        ::NPC_159_0007::
        btn = me:dialog(npc, '그럼 행운을 비네.. 아참, 이건 자네를 위해서 하는 얘기네만, 그 토끼는 아주 간악하다는 사실을 꼭 염두에 두게', { prev = true, next = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_159_0006
        end
    end
}
