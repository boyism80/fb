-- npc: 거북장군
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_RABBIT_HINT = 20
local MIN_LEVEL_RABBIT = 30
local RABBIT_LIVER_ITEM = '토끼의간'
local REWARD_ITEM = '주홍투구'

local MIN_LEVEL_SHARK_WEAPON = 99
local SHARK_WEAPON_ITEM_SCROLL = '무기제조법'
local SHARK_WEAPON_MATERIALS = { { name = '상어의핵', count = 3 }, { name = '불의수정', count = 3 } }
local SHARK_WEAPON_RESULT = '괴력선창'
local SHARK_WEAPON_SUCCESS_CHANCE = 60

local function run_rabbit_liver_quest(me, npc)
    local btn
    if me:level() < MIN_LEVEL_RABBIT then
        me:dialog(npc, '자네가 아직 이 일을 도우기엔 조금 역부족이라 생각이 되는군.', { prev = false, next = false })
        return
    end

    local q = me:quest(quest.QUEST_RABBIT_LIVER)
    if q == nil then
        ::NPC_144_0001::
        btn = me:dialog(npc, '휴우우.. 용왕님이 편찮으셔서 큰일입니다. 요즘 용궁은 용왕님이 편찮으셔서 다들 걱정하고 있답니다.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_144_0002::
        btn = me:dialog(npc, '들리는 소문엔 지상에 사는 토끼라는 동물의 간이 용왕님의 병에 좋은 효과가 있다고 하던데....', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_0001
        end
        ::NPC_144_0003::
        btn = me:dialog(npc, me:name() .. '님.. 용왕님을 위해서 토끼의 간을 구해다 주실 수 없겠습니까?', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_0002
        end
        ::NPC_144_0004::
        local sel, list_btn = me:list(npc, '구해주시겠습니까?', { '당연히 구해다 드려야죠.', '요즘 좀 바빠서..' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        q = me:start_quest(quest.QUEST_RABBIT_LIVER)
        if q == nil then
            return
        end
        q:step(1)
        me:push_achievement(ACHIEVEMENT_RABBIT_HINT, '토깽이의 행방을 알아보자.', 7, 1)
        ::NPC_144_0005::
        btn = me:dialog(npc, me:name() .. '님 정말 감사합니다.. 제발 토끼의 간을 구해 주세요.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_144_0006::
        btn = me:dialog(npc, '장터의 장돌뱅이가 토끼가 어디에 있는지 알고 있다고 합니다.. 그럼 부탁드립니다.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_144_0005
        end
        return
    end

    if q:completed() then
        me:dialog(npc, me:name() .. '님, 저번엔 정말 감사했습니다.', { prev = false, next = false })
        return
    end

    if not me:has_items(RABBIT_LIVER_ITEM, 1) then
        me:dialog(npc, '아직 토끼의간을 구하시지 못하셨군요..', { prev = false, next = false })
        return
    end
    
    ::NPC_144_0007::
    btn = me:dialog(npc, '토끼의간을 구해오셨군요! 용왕님의 병에 토끼의 간을 써도 될까요?', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_144_0008::
    local sel, list_btn = me:list(npc, '토끼의 간을 주시겠어요?', { '네, 가져가세요.', '아뇨.. 제가 가지고 있을래요.' }, { prev = false })
    if sel == nil or sel ~= 1 then
        return
    end
    local code = me:exchange(
        { ['item'] = { [RABBIT_LIVER_ITEM] = 1 } },
        { ['item'] = { [REWARD_ITEM] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST or code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        return
    end
    q:complete()
    me:push_achievement(ACHIEVEMENT_RABBIT_HINT, '거북장군의 부탁을 들어주었다.', 6, 1)
    ::NPC_144_0009::
    btn = me:dialog(npc, '아아.. 감사합니다. 이것으로 용왕님도 건강을 회복하실 수 있겠군요.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_144_0010::
    btn = me:dialog(npc, me:name() .. '님은 저희 용궁의 은인이십니다. 감사의 의미에서 투구를 하나 드리겠습니다.', { prev = true, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_144_0009
    end
    ::NPC_144_0011::
    btn = me:dialog(npc, '그럼 이 투구를 잘 쓰시길...', { prev = true, next = false })
    if btn == DIALOG_RESULT.PREV then
        goto NPC_144_0010
    end
end

local function run_shark_weapon_quest(me, npc)
    if me:level() < MIN_LEVEL_SHARK_WEAPON then
        me:dialog(npc, '자네가 아직 이 일을 도우기엔 조금 역부족이라 생각이 되는군.', { prev = false, next = false })
        return
    end

    local q = me:quest(quest.QUEST_SHARK_WEAPON)
    if q == nil then
        if me:dialog(npc, '상어장군이 사용하던 무기의 제작법이 드디어 알려졌다는 소문이 있다네...\n\n그 무기의 제작 방법을 아는 사람이 았다는 것 같더군.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '다시 상어장군용 무기를 만들기 위해서 그 제작방법이 꼭 필요하다네.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '자네가 좀 알아봐 줄 수 있는가?', { '물론입니다...', '별로 흥미가 안내켜서..' }, { prev = false })
        if sel == nil or sel ~= 1 then
            return
        end
        q = me:start_quest(quest.QUEST_SHARK_WEAPON)
        if q == nil then
            return
        end
        if q then
            q:step(1)
        end
        me:push_achievement(24, '상어장군의무기 퀘스트를 받다.', 7, 1)
        if me:dialog(npc, '좋아. 그런데 사실은 나도 누가 알고 있는지는 자세히는 모른다네.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, '단지 지나가는 말로 아주 추운 지방에서 누군가 알고 있다고 들은 기억이 있어서 말이야..\n\n자세한 건 자네가 직접 찾아봐야 할거야...\n\n그럼 부탁하네.', { prev = false, next = false })
        return
    end

    local step = q:step()
    if step >= 1 and step <= 10 then
        me:dialog(npc, '아주 추운곳에 있다고 들었는데...', { prev = false, next = false })
        return
    end

    if step == 11 then
        if not me:has_items(SHARK_WEAPON_ITEM_SCROLL, 1) then
            me:dialog(npc, '아직 아무것도 얻어오지 못했나?', { prev = false, next = false })
            return
        end
        if me:dialog(npc, '어디 그럼 한번 볼까.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '으음...으으음....으으으으음.....!!!\n\n아. 이제야 제조법을 알았네.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
            return
        end
        if me:dialog(npc, '이걸 만들기 위해서 상어의핵 3개와 불의수정 3개가 필요하다네.\n\n자네가 재료를 다 모아 온다면 내가 직접 만들어 주도록 하겠네.', { prev = false, next = false }) == DIALOG_RESULT.QUIT then
            return
        end
        if not me:rmitem(SHARK_WEAPON_ITEM_SCROLL, 1, ITEM_DELETE_TYPE.GIVE) then
            return
        end
        if q then
            q:step(12)
        end
        me:push_achievement(24, '상어장군 무기의 재료를 구하자.', 7, 1)
        return
    end

    if step == 12 then
        local ok = true
        for _, m in ipairs(SHARK_WEAPON_MATERIALS) do
            if not me:has_items(m.name, m.count) then
                ok = false
                break
            end
        end
        if not ok then
            me:dialog(npc, '재료가 부족한 것 같군. 상어의핵 3개와 불의수정 3개를 구해다 주시게.', { prev = false, next = false })
            return
        end
        local cost = { ['item'] = {} }
        for _, m in ipairs(SHARK_WEAPON_MATERIALS) do
            cost['item'][m.name] = m.count
        end
        local reward = nil
        if math.random(1, 100) <= SHARK_WEAPON_SUCCESS_CHANCE then
            reward = { ['item'] = { [SHARK_WEAPON_RESULT] = 1 } }
        end
        local code = me:exchange(cost, reward)
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 무기를 받을 수 없네.', { prev = false, next = false })
            return
        end
        if reward ~= nil then
            if q then
                q:step(13)
            end
            me:push_achievement(24, '상어장군 무기를 만들다!', 6, 1)
            if me:dialog(npc, '오. 드디어 만들었군...', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            if me:dialog(npc, '이것이 바로 괴력선창이라는 것일세.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, '이걸 자네에게 줄 터이니 부디 유용하게 사용하게나.', { prev = false, next = false })
        else
            me:dialog(npc, '이런.. 재료가 그만 모두 뭉개져버렸군..', { prev = false, next = false })
        end
        return
    end

    if step >= 13 then
        me:dialog(npc, '괴력선창은 유용하게 사용하고 있나?', { prev = false, next = false })
    end
end

return {
    ON_CLICK = function(me, npc)
        local sel = me:list(npc, '안녕하세요? 어떻게 오셨나요?', { '별주부전', '상어장군의무기' }, { prev = false })
        if sel == nil then
            return
        end
        if sel == 1 then
            run_rabbit_liver_quest(me, npc)
        else
            run_shark_weapon_quest(me, npc)
        end
    end
}
