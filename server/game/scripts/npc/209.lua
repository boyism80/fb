-- npc: 사천족제사장
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_HOLYTREE = 5
local COOLDOWN_SEC = 86400

local HOLYTREE_ITEMS = {
    { name = '백현목',   exp = 926000000 },
    { name = '자현목',   exp = 1353000000 },
    { name = '노송의가지', exp = 1550000000 },
    { name = '천년묵은가지', exp = 2100000000 },
}

function NPC_209(me, npc)
    if me:level() < 99 then
        me:dialog(npc, '99레벨 이상만 이용이 가능합니다.', { prev = false, next = false })
        return
    end

    local q = me:quest(quest.QUEST_HOLYTREE)
    if q == nil then
        q = me:start_quest(quest.QUEST_HOLYTREE)
        if q == nil then
            return
        end
    end

    local next_time = tonumber(q:param() or '') or 0
    local nt = now()

    if q:step() == 0 then
        if next_time > 0 and nt < next_time then
            me:dialog(npc, (next_time - nt) .. '초 후에 다시 오세요.', { prev = false, next = false })
            return
        end
        ::NPC_209_0001::
        local btn = me:dialog(npc, '최근 몇 년간 흉년이 계속되고 있어 마을을 위한 제를 지내기 위해 신성한 나무 가지를 구하고 있습니다.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_209_0010::
        btn = me:dialog(npc, '북방대초원에는 백현목,자현목,노송의가지,천년묵은가지와 같이 신성한 기운이 깃든 나무 가지들을 구할 수 있는데, 매일 매일 제를 지내야 하다 보니 많은 양의 나무 가지가 필요합니다.', { prev = true, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_209_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '저에게 신성한 기운이 깃든 나무를 가져다 주실 수 있을까요?', { '네. 구해보도록 할께요.', '아니요. 제가 좀 바빠서.' }, { prev = false })
        if sel == nil then
            return
        end
        if sel == 2 then
            goto NPC_209_0001
        end
        if sel == 1 then
            q:step(1)
            me:dialog(npc, '신성한 나무들은 북방대초원 21~30층을 돌아다니다 보면 구할 수 있을겁니다.\n\n그럼 부탁드리겠습니다.', { prev = false, next = true })
        end
        return
    end

    if q:step() == 1 then
        ::NPC_209_0020::
        local options = {}
        for i, t in ipairs(HOLYTREE_ITEMS) do
            options[i] = name_with(t.name, '을', '를') .. ' 가지고 왔습니다.'
        end
        local list2 = me:list(npc, '이렇게 열심히 제를 올리는데 언제쯤 흉년이 멈추려나..', options, { prev = false })
        if list2 == nil or list2 < 1 or list2 > #HOLYTREE_ITEMS then
            return
        end
        local item = HOLYTREE_ITEMS[list2]
        local confirm = me:list(npc, '풍년을 위한 제를 올릴 수 있도록 저에게 ' .. name_with(item.name, '을', '를') .. ' 주시겠습니까?', { '네. 드릴께요.', '아니요. 제가 쓸데가 있어서요.' }, { prev = false })
        if confirm == nil then
            return
        end
        if confirm ~= 1 then
            return
        end
        if me:group() ~= nil then
            me:dialog(npc, '그룹을 하신 상태로는 보상을 받을 수 없습니다.', { prev = false, next = true })
            return
        end

        local give_exp = math.floor(item.exp / exp_multiplier())
        local code = me:exchange(
            { ['item'] = { [item.name] = 1 } },
            { ['exp'] = give_exp }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, name_with(item.name, '이', '가') .. ' 없는데요?', { prev = false, next = false })
            return
        end

        q:param(tostring(nt + COOLDOWN_SEC))
        q:progress(q:progress() + 1)
        q:step(0)
        local count = q:progress()
        me:push_achievement(ACHIEVEMENT_HOLYTREE, '신성한나무를 ' .. count .. '회 가져다 주었다.', 7, count)
        me:dialog(npc, '감사합니다.\n\n하루 빨리 풍년이 찾아 올 수 있도록 몸과 마음을 다해 제를 올려야겠습니다.\n\n계속해서 잘 부탁드립니다.', { prev = false, next = false })
    end
end