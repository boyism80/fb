-- 무력화 캐스팅
local spell = require('lib.spell')

function ON_CAST_4012(me, you, sp)
    local mp = 500
    local sound = 85
    local effect = 10
    local error = me:assert(STATE.GHOST, STATE.RIDING)
    if error ~= nil then
        return me:message(error)
    end

    if me:mp() < mp then
        return me:message('마력이 부족합니다.')
    end
    me:mp_down(mp)

    local percent = 0.5 + ((me:int() / 255) * 0.5)
    local success = spell.boolean_random(percent)
    if not success then
        return me:message('무력화 실패')
    end

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(sp:name())))
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    if me ~= you and you:is(OBJECT_TYPE.CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(sp:name())))
    end
    for _, buff in pairs(you:buffs()) do
        you:unbuff(buff)
    end
end