local ACHIEVEMENT_MOUNTAIN_GOD = 41
local ITEM_FABRIC = '산신의비단'
local ITEM_NEEDLE = '산신의바늘'
local SPELL_MAP = '산신의보물지도'

local CLOTH_ITEMS = {
    '산신의천수', '산신의정화', '산신의인풍', '산신의비설',
    '산신의수령', '산신의정령', '산신의인령', '산신의비령',
}

local CLOTH_JOB = { '전사', '도적', '주술사', '도사', '전사', '도적', '주술사', '도사' }
local CLOTH_GENDER = { '남자', '남자', '남자', '남자', '여자', '여자', '여자', '여자' }

function NPC_138(me, npc)
    local quest = me:quest(QUEST_MOUNTAIN_GOD)

    if quest ~= nil and quest:completed() then
        me:dialog(npc, '제가 만들어 드린 산신의옷은 잘 사용하고 계신가요?', false, true)
        return
    end

    if quest == nil then
        ::COS001::
        local btn = me:dialog(npc, '안녕하세요~ 도삭산 800층까지 오시다니.. 수고 많으셨겠습니다.\n\n800층 까지도 물론 힘들게 오셨겠지만, 801층 부터는 완전히 다른 세계가 펼쳐진답니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::COS002::
        btn = me:dialog(npc, '가보시면 아시리라 생각합니다만, 원래부터 도삭산에 살고 있던 \'산신\'이라는 인간도 동물도 아닌 생물이 살고 있지요.\n\n신선수행을 하던 인간이 산신이 되었다는 얘기가 있는데, 사실인지는 알 수 없지요..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS001
        end
        ::COS003::
        btn = me:dialog(npc, '이들 산신은 인간이었을때 가지고 있던 소지품을 어떤곳에 묻어서 버린다고 하는데, 신선수행을 하던 사람들이었으니 귀중품들이 많이 있을겁니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS002
        end
        ::COS004::
        btn = me:dialog(npc, '도삭산 801층 이후에는 책장들이 있는 곳이 있는데, 그중 어딘가 한군데에서 제가 보물지도를 본적이 있습니다. 아마 산신들의 귀중품을 숨겨놓은 곳을 표시해둔 것 같은데, 그때 도망치느라 가져오질 못했습니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS003
        end
        local sel, list_btn = me:list(npc, '한번 구하러 가보시지요?', { '네, 찾아가봐야 겠군요.', '갈길이 바빠서..' }, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        if quest == nil then
            quest = me:start_quest(QUEST_MOUNTAIN_GOD)
            if quest == nil then
                me:dialog(npc, '퀘스트 시작 실패', false, true)
                return
            end
        end
        quest:step(1)
        me:push_achievement(ACHIEVEMENT_MOUNTAIN_GOD, '산신의보물지도를 찾자', 7, 25)
        me:dialog(npc, '그럼 보물을 찾으시면 저에게도 꼭 보여주셔야 됩니다!', false, true)
        return
    end

    local step = quest:step()
    if step == 1 then
        if not me:has_items('산신의보물지도', 1) then
            me:dialog(npc, '아직 보물을 찾기 못하신 것 같군요. 801층 이후에는 책장들이 있는 곳이 있는데, 그중 어딘가 한군데에서 제가 보물지도를 본적이 있습니다. 아마 산신들의 귀중품을 숨겨놓은 곳을 표시해둔 것 같은데...', false, false)
            return
        end
        me:dialog(npc, '오오! 보물지도를 구하셨군요! 어서 빨리 보물을 찾아보시지요!', false, true)
        quest:step(2)
        return
    end

    if step == 2 then
        if not me:has_items(ITEM_FABRIC, 1) then
            me:dialog(npc, '보물지도를 사용하면 숨겨진 곳이 적혀 있습니다. 그곳으로 가보세요.', false, false)
            return
        end
        local btn = me:dialog(npc, '오오! 이것은 산신의비단이 아닙니까! 이걸 저에게 맡기시면 제가 멋진 옷을 만들어드리겠습니다!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, list_btn = me:list(npc, '저에게 맡기시는 것이 어떠실지?', { '네 그럼 부탁합니다.', '안됩니다 어떻게 구한건데.' }, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        ::COS006::
        btn = me:dialog(npc, '크.. 이런 옷감을 내 손으로 만지게 되다니.. 영광입니다 영광..', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::COS007::
        btn = me:dialog(npc, '그런데 이 옷감.. 너무 튼튼해서 보통 바늘로는 꿰멜수가 없겠는데요.. 아무래도 산신들이 이 옷감을 다룰때 쓰는 바늘이 있을 것 같은데..\n\n산신들이 사용하는 바늘이 있어야 옷을 만들 수 있겠습니다..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS006
        end
        me:rmitem(ITEM_FABRIC, 1, ITEM_DELETE_TYPE.GIVE)
        quest:step(3)
        me:push_achievement(ACHIEVEMENT_MOUNTAIN_GOD, '산신의바늘을 구해 마석주에게 가져다주자.', 7, 25)
        return
    end

    if step == 3 then
        if not me:has_items(ITEM_NEEDLE, 1) then
            me:dialog(npc, '산신의바늘을 아직 구하지 못하신거 같은데요?', false, false)
            return
        end
        ::COS007B::
        local btn = me:dialog(npc, '이것은... 바늘이 아니라 새털처럼 보이는데.. 어디 한번..', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::COS008::
        btn = me:dialog(npc, '아니! 이것은 마법이 걸린 깃털인가 봅니다. 어째서 이런 튼튼한 옷감이 이런 부드러운 것에 구멍이 나는 것인지...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS007B
        end
        btn = me:dialog(npc, '자.. 그럼 어디한번 옷을 만들어볼까요.. 아. 그런데 어떤 옷을 만들어드리지요?', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS008
        end
        local sel, list_btn = me:list(npc, '어떤 옷을 만들어드릴까요?', {
            '산신의천수를 만들어주십시오',
            '산신의정화를 만들어주십시오',
            '산신의인풍을 만들어주십시오',
            '산신의비설을 만들어주십시오',
            '산신의수령을 만들어주십시오',
            '산신의정령을 만들어주십시오',
            '산신의인령을 만들어주십시오',
            '산신의비령을 만들어주십시오'}, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil or sel < 0 or sel > 7 then
            return
        end
        local item_name = CLOTH_ITEMS[sel + 1]
        local job = CLOTH_JOB[sel + 1]
        local gender = CLOTH_GENDER[sel + 1]
        ::COS009::
        btn = me:dialog(npc, name_with(item_name, '는', '은') .. ' ' .. gender .. ' ' .. job .. '들을 위한 옷입니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::COS010::
        btn = me:dialog(npc, '자.. 그럼.. ...... ...... ..........', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto COS009
        end
        local code = me:exchange(
            { ['item'] = { [ITEM_NEEDLE] = 1 } },
            { ['item'] = { [item_name] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '산신의바늘을 아직 구하지 못하신거 같은데요?', false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(item_name, '을', '를') .. ' 받을 수 없습니다.', false, false)
            return
        end
        quest:complete()
        me:push_achievement(ACHIEVEMENT_MOUNTAIN_GOD, '산신의옷을 만들다!', 6, 25)
        me:dialog(npc, '여기 있습니다! 멋진 옷을 만들게 되어 정말 기쁘군요!', false, true)
        return
    end
end