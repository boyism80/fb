-- mob: 전욱
local quest = require('lib.quest')

return {
    ON_MOB_ATTACK = function(me, you)
        return false
    end,

    -- ON_MOB_DIE = function(me)
    -- end,

    ON_MOB_KILL = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_MUTA)
        if q == nil or q:step() ~= 2 then
            return
        end
        me:mkitem('전욱의증표', 1)
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
