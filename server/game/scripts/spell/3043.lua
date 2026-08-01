-- spell: 헬파이어
-- 헬파이어 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        if me:is(OBJECT_TYPE.CHARACTER) then
            local weapon = me:weapon()
            local delay = 7
            if weapon ~= nil then
                local name = weapon:name()
                if name == '용랑제삼봉' then
                    delay = delay - 1
                elseif name == '용랑제사봉' then
                    delay = delay - 1
                elseif name == '용랑제오봉' then
                    delay = delay - 2
                elseif name == '용랑제육봉' then
                    delay = delay - 2
                elseif name == '용랑제칠봉' then
                    delay = delay - 3
                elseif name == '용랑제팔봉' then
                    delay = delay - 3
                elseif name == '용랑제구봉' then
                    delay = delay - 4
                end
            end
            me:spell(sp):delay(delay)
        end

        local damage = math.floor(me:mp() * 1.5)
        local mp = me:mp()
        local sound = 73
        local effect = 8
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
