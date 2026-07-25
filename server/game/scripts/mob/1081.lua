-- mob: 게장군
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
        local mob = mobs[1]
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        local q = me:quest(quest.QUEST_DRAGON_KING)
        if q == nil or q:step() ~= 3 then
            return
        end

        local sel, list_btn = me:list(mob, "으...분하다...네놈에게 지다니...", { "이놈, 나의 포박을 받아라", "용왕님이 너를 보자하신다.", "따라오너라" }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil or sel < 1 or sel > 3 then
            return
        end

        q:progress(1)
        me:push_achievement(19, "게장군을 생포했다!", 7, 1)
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
