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
    	you:target(nil)
    	you:buff(spell, buff_time, me)
    end
end

function on_uncast(me, spell)
    me:oblivion(nil)
    unbuff(me, spell)
end