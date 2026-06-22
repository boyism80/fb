-- npc: 도태연
local quest = require('lib.quest')
local server = require('lib.server')

local ACHIEVEMENT_400 = 12
local REQUIRED_ITEMS = {
    ['도삭산코뿔소발톱'] = 1,
    ['도삭산코뿔소이빨'] = 1,
    ['도삭산코뿔소뿔'] = 1,
}
local REWARD_400 = '도삭산황금봉'

function NPC_191(me, npc)
    local q = me:quest(quest.QUEST_DOTAEYEON)

    if q == nil then
        ::NPC_191_0001::
        local btn = me:dialog(npc, '안녕하시오~ 도삭산코뿔소말이오. 그놈 이빨과 발톱, 뿔 정말 단단하게 생기지 않았소?', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_191_0010::
        btn = me:dialog(npc, '코뿔소의 이빨,발톱,뿔은 무기나 갑옷을 만드는데 정말 좋은 재료가 되지. 그중에서도 도삭산코뿔소는 내가 지금까지 본 것들중에서 제일이더군.\n\n그래서 요즘 도삭산코뿔소를 사냥하고 있는데 놈들이 워낙 사나워서 사냥이 매우 위험하다오.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_191_0001
        end
        ::NPC_191_0011::
        btn = me:dialog(npc, '어떻소 도삭산코뿔소를 사냥해서 이빨,발톱,뿔을 내게 구해다 주지 않겠소? 물론 그냥 가져다 달라는것은 아니니 걱정 마시오. 내 좋은 물건 하나 주도록하지!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_191_0010
        end
        local sel, list_btn = me:list(npc, '내 제안이 어떻소?', { '좋습니다. 맘에 드는군요.', '죄송하지만 전 갈길이 바빠서..' }, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        if q == nil then
            q = me:start_quest(quest.QUEST_DOTAEYEON)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', false, true)
                return
            end
        end
        q:step(1)
        me:push_achievement(ACHIEVEMENT_400, '도태연의 부탁을 들어주자!', 7, 1)
        me:dialog(npc, '다시 한번 말하지만 도삭산코뿔소의 발톱,이빨,뿔 모두 구해다 줘야하오~\n\n하나라도 빠지면 내 선물을 줄 수 없으니 명심하게나. 그럼 몸조심하구려. 놈들이 정말 사납거든.', false, true)
        return
    end

    local step = q:step()
    if q:completed() or step > 1 then
        me:dialog(npc, '안녕하신가? 덕분에 아무 훌륭한 무기와 방어구를 제작했다네. 나중에 한번 구경 오시게나.', false, true)
    end

    if step == 1 then
        ::NPC_191_0020::
        local btn = me:dialog(npc, '어디보자~ 코뿔소 사냥은 많이 했소? 발톱,이빨,뿔 다 가지고 왔는지 봅시다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not me:has_items(REQUIRED_ITEMS) then
            me:dialog(npc, '아직 발톱, 이빨, 뿔을 다 구하지 못한 모양이군..', false, false)
            return
        end
        btn = me:dialog(npc, '오! 세가지 다 구해왔구려. 역시 내가 사람보는 눈은 정확하다니깐. 허허~\n\n그러면 내가 다 가져가겠소.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = REQUIRED_ITEMS },
            { ['item'] = { [REWARD_400] = 1 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 발톱, 이빨, 뿔을 다 구하지 못한 모양이군..', false, false)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(REWARD_400, '을', '를') .. ' 받을 수 없습니다.', false, false)
            return
        end
        me:push_achievement(ACHIEVEMENT_400, '도삭산 400층 퀘스트 완료', 7, 1)
        q:step(2)
        btn = me:dialog(npc, '자 여기 내가 주는 선물이오. 도삭산의 비밀에 관련된 것 같은데.. 언젠가 쓸 일이 있을 것이오.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end
end
