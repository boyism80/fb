-- spell: 귀염추혼소
-- 귀염추혼소 캐스팅
local spell = require('lib.spell')

function ON_CAST_4021(me, sp)
    local mp = 600
    local sound = 79
    local effect = 39

    for _, obj in pairs(me:nears(OBJECT_TYPE.LIFE)) do
        if not obj:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
            obj:effect(effect)
            obj:buff(sp, 425, me)
            if me ~= obj and obj:is(OBJECT_TYPE.CHARACTER) then
                obj:message(string.format('%s님이 %s 걸었습니다.', me:name(), name_with(sp:name())))
            end
        end
    end
    me:sound(sound)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
end

-- 귀염추혼소 버프 효과
function ON_BUFF_4021(me, sp)
    me:buff_phydef(me:buff_phydef() + 50)
end

-- 귀염추혼소 버프 해제 효과
function ON_UNBUFF_4021(me, sp)
    me:buff_phydef(me:buff_phydef() - 50)
end
