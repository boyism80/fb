-- 백호의희원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4037(me, you, sp)
    local hp = me:mp()*2
    local mp = me:mp()//2
    local sound = 95
    local effect = 64
    local delay = math.min(60, me:maxmp() // 10000)
    if me:is(OBJECT_TYPE.CHARACTER) then
        local weapon = me:weapon()
        if weapon ~= nil then
            local name = weapon:name()
            if name == '용겸제삼봉' then
                delay = delay - (delay * 0.1)
            elseif name == '용겸제사봉' then
                delay = delay - (delay * 0.15)
            elseif name == '용겸제오봉' then
                delay = delay - (delay * 0.2)
            elseif name == '용겸제육봉' then
                delay = delay - (delay * 0.25)
            elseif name == '용겸제칠봉' then
                delay = delay - (delay * 0.3)
            elseif name == '용겸제팔봉' then
                delay = delay - (delay * 0.35)
            elseif name == '용겸제구봉' then
                delay = delay - (delay * 0.4)
            end
        end
    end
    me:spell(sp):delay(delay)
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end