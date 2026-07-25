-- mob: 요제
local quest = require('lib.quest')

return {
    on_mob_attack = function(me, you)
        return false
    end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_MUTA)
        if q == nil or q:step() ~= 4 then
            return
        end
        me:mkitem('요제의증표', 1)
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
