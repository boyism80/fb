-- mob: 복어장군
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

        mob:chat('asd')
        local q = me:quest(quest.QUEST_DRAGON_KING)
        if q == nil or q:step() ~= 1 or q:progress() < 1 then
            return
        end
        local item_doc = "태자전음문서"
        if me:has_items(item_doc, 1) then
            return
        end

        me:mkitem(item_doc, 1)
        me:push_achievement(19, "복어장군을 생포하였다!", 7, 1)
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
