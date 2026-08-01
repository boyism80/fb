-- npc: 준준
local quest = require('lib.quest')

local function cultural_property_quest(me, npc)
    local totem_names = { '번개의토템', '바람의토템', '대지의토템', '화염의토템' }
    local reward_at = { 5, 10, 15 }
    local reward_items = { '선장의일기5', '반룡곤', '가시나무봉' }

    local q = me:quest(quest.QUEST_JUNJUN)
    local lighthouse = me:quest(quest.QUEST_LIGHTHOUSE)

    if q == nil then
        if lighthouse == nil or lighthouse:completed() or lighthouse:step() ~= 5 then
            me:dialog(npc, '....', { prev = false, next = false })
            return
        end
        ::NPC_115_0001::
        local button = me:dialog(npc, '외부와 교류를 시작하면서 가장 힘든 게 뭔지 아나? 내부의 적과 싸우는 것이야. 요즘 우리의 소중한 문화재를 파내서 팔아먹는 비류배들이 있어서 말이야, 막을 방법이 없어 골치가 아프다~ 휴우~', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_115_0002::
        local selected, list_btn = me:list(npc, '자네처럼 외부인 대표로, 우리가 유출된 문화재를 찾아서 나에게 가져다 주지 않겠나? 외부인에 대한 우리의 불신을 풀어줄 좋은 기회가 될거야.', { '네, 구해 보겠습니다.', '글쎄요...' }, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_115_0001
        end
        if selected ~= 1 then
            return
        end
        q = me:start_quest(quest.QUEST_JUNJUN)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
            return
        end
        q:progress(0)
        ::NPC_115_0003::
        local last_btn = me:dialog(npc, '그럼 부탁하네. 주로 토템들이 도굴당하고 있는데.. 도굴꾼 들은 내가 잡는다쳐도, 아예 어디다 떨구고 잃어버린 것까지 찾아다닐 시간이 없어. \n\n토템들을 좀 찾아다 줬음 좋겠군.', { prev = false, next = true })
        if last_btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if q:completed() then
        me:dialog(npc, '일은 정말 힘들군!', { prev = false, next = true })
        return
    end

    local selected = me:list(npc, '일은 열심히 하고 있는지 모르겠군. 덥겠지만, 분발해주게나!', { '토템을 하나 찾아왔습니다', '보상을 요구한다.', '그냥 들려본건데요.' })
    if selected == nil then
        return
    end

    if selected == 1 then
        local totem_sel = me:list(npc, '그래..어떤 토템을 가지고 왔는가?', { '번개의토템', '바람의토템', '대지의토템', '화염의토템' })
        if totem_sel == nil or totem_sel < 1 or totem_sel > #totem_names then
            return
        end
        local item_name = totem_names[totem_sel]
        if not me:has_items(item_name, 1) then
            me:dialog(npc, name_with(item_name, '이', '가') .. ' 없는데?', { prev = false, next = true })
            return
        end
        me:rmitem(item_name, 1, ITEM_DELETE_TYPE.GIVE)
        q:inc_progress(1)
        me:push_achievement(33, string.format('문화재보호공무원에게 토템을 %d개 가져다 주었다.', q:progress()), 7, 1)
        me:dialog(npc, '수고했네. 계속 힘내주게.', { prev = false, next = true })
        return
    end

    if selected == 2 then
        local progress = q:progress()
        local step = q:step()
        local next_tier = nil
        for i = 1, #reward_at do
            if progress >= reward_at[i] and step < i then
                next_tier = i
                break
            end
        end
        if next_tier == nil then
            me:dialog(npc, '아직 한 일이 별로 없는거 같은데? 토템을 적어도 5번은 가져와야 보상을 줄 수 있네.', { prev = false, next = true })
            return
        end
        local reward_name = reward_items[next_tier]
        if me:mkitem(reward_name, 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(reward_name, '을', '를') .. ' 줄 수 없네.', { prev = false, next = true })
            return
        end
        q:step(next_tier)
        if next_tier == 3 then
            q:complete()
            if lighthouse ~= nil and not lighthouse:completed() then
                lighthouse:step(6)
            end
            me:push_achievement(32, '진백랑의 부탁을 들어주자. [6/6]', 7, 1)
        end
        me:dialog(npc, '수고했네. 계속 힘내주게.', { prev = false, next = true })
        return
    end    
end

return {
    on_click = function(me, npc)
        local OPT_CULTURAL = '문화재보호공무원'
        local OPT_MONSTER = '환상의섬에 나타난 괴물'
        local selected, button = me:pursuit(npc, '안녕하세요. 어떻게 오셨나요?', { OPT_CULTURAL, OPT_MONSTER })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if selected == OPT_CULTURAL then
            cultural_property_quest(me, npc)
        end
    end
}
