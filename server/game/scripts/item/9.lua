-- item: 도톨

local quest = require('lib.quest')

return {
    on_activated = function(me, item)
        local q = me:quest(quest.QUEST_NAKRANG2)
        if q and not q:completed() and q:step() == 1 and q:progress() < 2 then
            q:progress(q:progress() + 1)
        end
        me:hp(math.min(me:hp() + 8, me:maxhp()))
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
