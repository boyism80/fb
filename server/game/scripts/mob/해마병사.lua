local quest = require('lib.quest')

function ON_MOB_DIE_1085(me, you)
    if you == nil or me == nil then
        return
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    local q = you:quest(quest.QUEST_DRAGON_KING)
    if q == nil or q:step() ~= 7 then
        return
    end

    local p = q:progress()
    if p < 500 then
        q:progress(p + 1)
    end
end