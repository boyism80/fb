local quest = require('lib.quest')

function NPC_188(me, npc)
    local q = me:quest(quest.QUEST_SAILOR)

    if q ~= nil and q:step() == 3 then
        me:dialog(npc, '결국 왔구만! 어때, 와 보니 정말 좋은 곳이지 않은가? 허허... 난 여기서 바닷바람을 쐬면서 좀 쉬어야겠네. 낮잠자기 딱 좋은 곳이구만.', false, true)
        q:step(4)
        return
    end

    if q ~= nil and q:step() == 7 then
        me:dialog(npc, '또 왔구만! 허허, 난 지금 여기 특산품을 잔뜩 먹고 배가 불러서 잠시 쉬고 있는 중일세. 바닷바람을 쐬고 있으려니 참 시원하구만. 허허...', false, true)
        if me:mkitem('정령인형', 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 정령인형을 줄 수 없네.', false, true)
            return
        end
        q:step(8)
        me:dialog(npc, '그렇지, 이렇게 계속 만나는 것도 인연인데 그냥 보내긴 섭섭하구만, 이건 여기 특산품일세. 신비한 힘이 느껴지는 인형이지. 선물용으로 좋으려나? 허허... 그럼 난 함숨 자봐야겠네. 즐거운 여행 하시게나...', false, true)
        return
    end

    me:dialog(npc, '....', false, false)
end