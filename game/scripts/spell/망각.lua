function on_cast(me, you, spell)
    if not you:is(OBJECT_TYPE_MOB) then
        return me:message('걸리지 않습니다.')
    end

    local mp = 60
    local sound = 32
    local effect = 34
    local buff_time = 4
    if buff_cast(me, you, spell, mp, sound, effect) then
        you:oblivion(me)
    	you:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)
    me:target(nil)
end

function on_unbuff(me, spell)
    me:oblivion(nil)
end