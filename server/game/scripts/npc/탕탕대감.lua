local quest = require('lib.quest')
local server = require('lib.server')

function NPC_120(me, npc)
    local q = me:quest(quest.QUEST_TANGTANG)
    local lighthouse = me:quest(quest.QUEST_LIGHTHOUSE)

    if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 4 then
        me:dialog(npc, '....', false, false)
        return
    end

    if q == nil then
        local btn = me:dialog(npc, '요즘 대륙 사람들이 자꾸 눈에 띄눈군. 정말 성가신 일이야. 그러고보니 자네도 대륙 사람이지? 그렇다면 태존도에 대해 알겠군?', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local selected = me:list(npc, '다른 나라의 무기를 연구중인데, 그 태존도가 좀 필요해. 구해다주겠나?', { '네, 구해다 드리겠습니다.', '싫습니다.' })
        if selected == nil or selected ~= 0 then
            return
        end

        local q = me:start_quest(quest.QUEST_TANGTANG)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:dialog(npc, '나름대로 보답은 하겠어. 좀 서둘러만 줬음 좋겠군.', false, true)
        return
    end

    if q:completed() then
        me:dialog(npc, '태존도.. 정말 신기한 무기군.', false, true)
        return
    end

    if not me:has_items('태존도', 1) then
        me:dialog(npc, '아직 태존도를 구하지 못한건가?', false, true)
        return
    end

    local btn = me:dialog(npc, '음, 그게 태존도인가? 이리 주시게.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ['태존도'] = 1 } },
        { ['item'] = { ['선장의일기4'] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, '아직 태존도를 구하지 못한건가?', false, true)
        return
    elseif code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, '소지품이 가득 차서 선장의일기4를 줄 수 없네.', false, true)
        return
    end
    q:complete()
    lighthouse:step(5)
    me:push_achievement(32, '진백랑의 부탁을 들어주자. [5/6]', 7, 1)
    me:dialog(npc, '고맙다는 말은 하지 않겠네. 이건 거래니까. 자, 가져가게', false, true)
end