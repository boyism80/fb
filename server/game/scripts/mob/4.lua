-- mob: 삽사리
local quest = require('lib.quest')

return {
    -- ON_MOB_ATTACK = function(me, you)
    -- end,

    -- ON_MOB_DIE = function(me)
    -- end,

    ON_MOB_KILL = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        local q = me:quest(quest.QUEST_NAKRANG_HUNT)
        if q ~= nil then
            if mobs[1]:model():name() == q:param() then
                q:inc_progress(#mobs)
            end
        end
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
