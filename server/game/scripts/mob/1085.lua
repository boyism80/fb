-- mob: 해마병사
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
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_DRAGON_KING)
        if q == nil or q:step() ~= 7 then
            return
        end

        local p = q:progress()
        if p < 500 then
            q:progress(math.min(500, p + #mobs))
        end
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
