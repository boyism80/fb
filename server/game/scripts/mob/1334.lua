-- mob: 꿩
local festival = require('lib.festival')

local FEATHER = '꿩의깃털'

local function try_feather_drop(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('석가탄신일') then
        return
    end

    if you:role() < ROLE.ADMIN then
        math.randomseed(seed())
        if math.random(1, 2) ~= 1 then
            return
        end
    end

    if you:mkitem(FEATHER, 1) == nil then
        you:message("소지품이 가득 차서 깃털을 받을 수 없습니다.")
    end
end

return {
    on_mob_damaged = function(me, you)
        try_feather_drop(me, you)
    end,

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
