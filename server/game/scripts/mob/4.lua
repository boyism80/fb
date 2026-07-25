-- mob: 삽사리
local quest = require('lib.quest')

return {
    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
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

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
