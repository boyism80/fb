-- mob: 꿩
local festival = require('lib.festival')

return {
    on_mob_attack = function(me, you)
        if me == nil then
            return
        end

        local hp = me:hp()
        local maxhp = me:maxhp()
        if hp < maxhp then
            me:hp(math.min(hp + 20, maxhp))
        end

        local dirs = { DIRECTION.LEFT, DIRECTION.TOP, DIRECTION.RIGHT, DIRECTION.BOTTOM }
        local dir = dirs[math.random(1, 4)]
        me:direction(dir)
        local fx, fy = me:front_position(1)
        if fx ~= nil and fy ~= nil then
            me:position(fx, fy)
        end
    end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('석가탄신일') then
            return
        end
        festival.set_pheasant_kill_penalty(me, 3600)
        me:message("살생을 하였습니다. 한동안 연등을 만들 수 없습니다.")
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
