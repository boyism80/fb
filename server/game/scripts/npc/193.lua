-- npc: 우가풍
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_600 = 14
local REQUIRED_ITEM = { ['어머니의호롱불'] = 1 }
local REWARD_600 = '수정해골'

function NPC_193(me, npc)
    local q = me:quest(quest.QUEST_WOOGAPUNG)

    if q == nil then
        ::NPC_193_0001::
        local btn = me:dialog(npc, '내 호롱불이..내 호롱불..', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_193_0002::
        btn = me:dialog(npc, '내 소중한 호롱불을 도삭산문비 녀석이 훔쳐갔다네. 그 호롱불은 어머니가 물려주신 유일한 물건이지, 어느새 그것없이는 잠을 이룰 수가 없게 되었다네.\n\n아늑한 호롱불빛이 따뜻한 어머니의 손길처럼 느껴져서...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_193_0001
        end
        ::NPC_193_0003::
        btn = me:dialog(npc, '신기한것은 그런 무리가 얼마전까진 없었는데 요즘들어 생겨나서는 말썽을 부린다는 점이지. 원래 도삭산은 살기좋은 곳이었는데.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_193_0002
        end
        local sel, list_btn = me:list(npc, '부탁이네. 호롱불을 찾아다 줄 수 없겠나? 나에겐 정말 소중한 물건이라네.', { '알겠습니다. 제가 찾아다드리죠.', '호롱불은 얼마든지 많은데...' }, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 2 then
            me:dialog(npc, '그렇지만.. 그게 잘 안된다네..', false, false)
            return
        end
        if sel ~= 1 then
            return
        end
        q = me:start_quest(quest.QUEST_WOOGAPUNG)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        q:step(1)
        me:push_achievement(ACHIEVEMENT_600, '우가풍의 부탁을 들어주자!', 7, 1)
        ::NPC_193_0005::
        btn = me:dialog(npc, '정말 고맙네. 그 마음만으로도. 문비는 무슈후슈들이 나오는 곳을 지나면 다른 녀석들과 함께 무리로 살고 있지.\n\n호롱불을 훔쳐간 녀석은 뭔가 좀 특이했네. 겉보기엔 분간이 잘 안되지만 문비 소굴을 다니다 보면 분명 만날수 있을거야.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_193_0006::
        btn = me:dialog(npc, '꼭 그 못된 놈을 잡아서 호롱불을 찾아주게. 휴우.. 자네밖에 믿을 사람이 없네. 그럼..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_193_0005
        end
        return
    end

    if q:completed() then
        me:dialog(npc, me:name() .. '씨 아닌가? 저번엔 정말 고마웠네. 덕분에 잠을 편안히 잘 수 있게 되었다네.', false, true)
        return
    end

    local step = q:step()
    if step == 1 then
        local code = me:exchange(
            { ['item'] = REQUIRED_ITEM },
            { ['item'] = { [REWARD_600] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 찾으시지 못하신것 같군. 문비는 무슈후슈들이 나오는 곳을 지나면 다른 녀석들과 함께 무리로 살고 있지.', false, false)
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(REWARD_600, '을', '를') .. ' 받을 수 없습니다.', false, false)
            return
        end
        q:complete()
        me:push_achievement(ACHIEVEMENT_600, '도삭산 600층 퀘스트 완료', 7, 1)
        ::NPC_193_0007::
        btn = me:dialog(npc, '이..이것은... 맞네! 어머니의 호롱불이 맞아! 정말 어떻게 감사의 말을 해야할지. 감사히 받겠소.\n\n대단하구려. 그녀석을 잡다니. 잠깐잠깐 이럴때가 아니지. 내 꼭 주고 싶은것이 있었는데.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_193_0008::
        btn = me:dialog(npc, '얼마전에 도삭산 여행을 다니다가 주운것인데 상당히 희귀한것이라네. 도삭산의 비밀에 관계된 것이 아닐까 하는데..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_193_0007
        end
        ::NPC_193_0009::
        btn = me:dialog(npc, '자네덕분에 오늘부터 잠을 편안히 잘 수 있겠군. 고마워~', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_193_0008
        end
        return
    end

    me:dialog(npc, me:name() .. '씨 아닌가? 저번엔 정말 고마웠네. 덕분에 잠을 편안히 잘 수 있게 되었다네.', false, true)
end