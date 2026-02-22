local ACHIEVEMENT_RABBIT_HINT = 20
local MIN_LEVEL_RABBIT = 30
local RABBIT_LIVER_ITEM = '토끼의간'
local REWARD_ITEM = '주홍투구'

local function run_rabbit_liver_quest(me, npc)
    local btn
    if me:level() < MIN_LEVEL_RABBIT then
        me:dialog(npc, '자네가 아직 이 일을 도우기엔 조금 역부족이라 생각이 되는군.', false, false)
        return
    end

    local quest = me:quest(QUEST_RABBIT_LIVER)
    if quest == nil then
        ::NPC_144_COS001::
        btn = me:dialog(npc, '휴우우.. 용왕님이 편찮으셔서 큰일입니다. 요즘 용궁은 용왕님이 편찮으셔서 다들 걱정하고 있답니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_144_COS002::
        btn = me:dialog(npc, '들리는 소문엔 지상에 사는 토끼라는 동물의 간이 용왕님의 병에 좋은 효과가 있다고 하던데....', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_COS001
        end
        ::NPC_144_COS003::
        btn = me:dialog(npc, me:name() .. '님.. 용왕님을 위해서 토끼의 간을 구해다 주실 수 없겠습니까?', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_COS002
        end
        ::NPC_144_COS004::
        local sel, list_btn = me:list(npc, '구해주시겠습니까?', { '당연히 구해다 드려야죠.', '요즘 좀 바빠서..' }, true)
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_144_COS003
        end
        if sel == nil or sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_RABBIT_LIVER) then
            return
        end
        quest = me:quest(QUEST_RABBIT_LIVER)
        if quest == nil then
            return
        end
        quest:step(1)
        me:push_achievement(ACHIEVEMENT_RABBIT_HINT, '토깽이의 행방을 알아보자.', 7, 1)
        ::NPC_144_COS005::
        btn = me:dialog(npc, me:name() .. '님 정말 감사합니다.. 제발 토끼의 간을 구해 주세요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_144_COS006::
        btn = me:dialog(npc, '장터의 장돌뱅이가 토끼가 어디에 있는지 알고 있다고 합니다.. 그럼 부탁드립니다.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_COS005
        end
        return
    end

    if quest:completed() then
        me:dialog(npc, me:name() .. '님, 저번엔 정말 감사했습니다.', true, false)
        return
    end

    if not me:has_items(RABBIT_LIVER_ITEM, 1) then
        me:dialog(npc, '아직 토끼의간을 구하시지 못하셨군요..', false, false)
        return
    end
    
    ::NPC_144_COS007::
    btn = me:dialog(npc, '토끼의간을 구해오셨군요! 용왕님의 병에 토끼의 간을 써도 될까요?', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_144_COS008::
    local sel, list_btn = me:list(npc, '토끼의 간을 주시겠어요?', { '네, 가져가세요.', '아뇨.. 제가 가지고 있을래요.' }, false)
    if sel == nil or sel ~= 0 then
        return
    end
    quest:complete()
    me:rmitem(RABBIT_LIVER_ITEM, 1, ITEM_DELETE_TYPE.GIVE)
    me:mkitem(REWARD_ITEM, 1)
    me:push_achievement(ACHIEVEMENT_RABBIT_HINT, '거북장군의 부탁을 들어주었다.', 6, 1)
    ::NPC_144_COS009::
    btn = me:dialog(npc, '아아.. 감사합니다. 이것으로 용왕님도 건강을 회복하실 수 있겠군요.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_144_COS010::
    btn = me:dialog(npc, me:name() .. '님은 저희 용궁의 은인이십니다. 감사의 의미에서 투구를 하나 드리겠습니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_144_COS009
    end
    ::NPC_144_COS011::
    btn = me:dialog(npc, '그럼 이 투루를 잘 쓰시길...', true, false)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_144_COS010
    end
end

local function run_shark_weapon_placeholder(me, npc)
    me:dialog(npc, '상어장군의 무기 관련 일은 아직 준비 중입니다.', false, false)
end

function NPC_144(me, npc)
    local sel = me:list(npc, '안녕하세요? 어떻게 오셨나요?', { '별주부전', '상어장군의무기' }, false)
    if sel == nil then
        return
    end
    if sel == 0 then
        run_rabbit_liver_quest(me, npc)
    else
        run_shark_weapon_placeholder(me, npc)
    end
end