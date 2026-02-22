local GOAL = 5

-- 토끼 사망
function ON_MOB_DIE_2(me, you)
    local quest = you:quest(QUEST_NAKRANG_HUNT)
    if quest ~= nil then
        if me:model():name() == quest:param() then
            quest:inc_progress()
        end
    end

    local q5 = you:quest(QUEST_NAKRANG5)
    if q5 == nil or q5:completed() or q5:step() ~= 1 then
        return
    end
    local param = q5:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min(tonumber(a) or 0, GOAL)
    local rabbit = math.min((tonumber(b) or 0) + 1, GOAL)
    q5:param(string.format('%d,%d', squirrel, rabbit))
end