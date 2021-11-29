function on_cast(me, spell)
    if me:isbuff(spell) then
        me:message('이미 걸려 있습니다.')
        return
    end

    me:action(ACTION_CAST_SPELL, 0x00, 0x1c)
    me:state(0x05)
    me:effect(0x14)
    me:buff(spell, 180)
    me:message('투명을 외웠습니다.')
end

function on_uncast(me, spell)
    me:state(0x00)
    me:message('투명 해제')
end