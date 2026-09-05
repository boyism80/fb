-- mob: 잡초
local quest = require('lib.quest')
local festival = require('lib.festival')

return {
    -- 설날 only: 50% chance to drop 잡초풀 x1-3 on the ground.
    on_mob_die = function(me)
        if me == nil then
            return
        end
        if not festival.is('설날') then
            return
        end
        if math.random(1, 100) > 50 then
            return
        end
        local n = math.random(1, 3)
        for _ = 1, n do
            me:mkitem('잡초풀')
        end
    end,

    on_mob_kill = function(me, mobs)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('추석') then
            return
        end

        local q = quest.get_annual(me, quest.QUEST_BAEKRIHYANG)
        if q == nil or q:completed() or q:step() < 1 then
            return
        end
        if q:progress() >= 1 then
            return
        end
        q:progress(1)
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
