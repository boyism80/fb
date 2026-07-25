-- item: 도톨

local quest = require('lib.quest')

return {
    ON_ACTIVATED = function(me, item)
        local q = me:quest(quest.QUEST_NAKRANG2)
        if q and not q:completed() and q:step() == 1 and q:progress() < 2 then
            q:progress(q:progress() + 1)
        end
        me:hp(math.min(me:hp() + 8, me:maxhp()))
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
