-- 정지 (id 15). Add logic as needed.
function ON_CAST_15(me, you, spell)
    if not you:is(OBJECT_TYPE.CHARACTER) then
        me:message('걸리지 않습니다.')
        return
    end

    if you:role() >= me:role() then
        me:message('걸리지 않습니다.')
        return
    end

    local mp = 80
    local sound = 70
    local effect = 1
    local buff_time = 20
    if debuff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_15(me, spell)
    me:freeze(true)
end

function ON_UNBUFF_15(me, spell)
    me:freeze(false)
end
