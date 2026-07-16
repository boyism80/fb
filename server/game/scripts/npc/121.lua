-- npc: 퉁퉁대감
local quest = require('lib.quest')
local enum = require('lib.enum')

function NPC_121(me, npc)
    local q = me:quest(quest.QUEST_TUNGTUNG)
    local lighthouse = me:quest(quest.QUEST_LIGHTHOUSE)

    if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 2 then
        me:dialog(npc, '....', false, false)
        return
    end

    if q == nil then
        local selected = me:list(npc, "대륙 사람이로군. 듣기에 대륙에는 '호박'이라는 예쁜 게 있다더군. 그거 열 개만 구해다 줄 수 있을까?", { '네, 구해드리지요.', '아니오, 바빠서..' })
        if selected == nil or selected ~= 1 then
            return
        end
        local q = me:start_quest(quest.QUEST_TUNGTUNG)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:dialog(npc, '내 좋은걸로 보답할테니 어서 다녀오게. 잊지 말게. 호박 열 개 일세.', false, true)
        return
    end

    if q:completed() then
        me:dialog(npc, '호박.. 참 예쁘군..', false, true)
        return
    end

    if not me:has_items('호박', 10) then
        me:dialog(npc, '아직 호박 열 개를 구하지 못했군.', false, true)
        return
    end

    local btn = me:dialog(npc, '잘 가져왔군. 이리주게.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ['호박'] = 10 } },
        { ['item'] = { ['선장의일기2'] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아직 호박 열 개를 구하지 못했군.', false, true)
        return
    end
    if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 선장의일기2를 줄 수 없네.', false, true)
        return
    end
    q:complete()
    lighthouse:step(3)
    me:push_achievement(32, '진백랑의 부탁을 들어주자. [3/6]', 7, 1)
    me:dialog(npc, '음, 예쁘긴 예쁘군. 약속대로 좋은걸 주지.', false, true)
end