function on_cast(me, you, spell)
    if you:isbuff('백열장') then
        me:message('이미 걸려 있습니다.')
        return
    end

    me:action(ACTION_CAST_SPELL, 0x00, 0x1c)
    me:state(0x05)
    me:effect(0x14)
    me:buff(spell, 5)
    me:message('백열장을 외웠습니다.')
    if(me ~= you) then
        you:message(string.format('%s님이 백열장을 외워주셨습니다.', me:name()))
    end
end

function on_uncast(me, spell)

end