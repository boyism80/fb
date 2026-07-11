-- mob: 해파리부하
local quest = require('lib.quest')

function ON_MOB_KILL_1007(me, mobs)
    if me == nil or mobs == nil or #mobs == 0 then
        return
    end
    local mob = mobs[1]
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = me:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 16 then
        return
    end

    if math.random(1, 100) > 10 then
        return
    end

    if me:has_items("전략문서", 1) then
        return
    end

    local item = me:mkitem("전략문서", 1)
    me:dialog(item, "휴.. 드디어 전략문서를 구했다.", false, true)
end

-- function ON_MOB_DIE_1007(me)
-- end
