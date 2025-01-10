function on_cast(me, you, spell)
    if not spell_cast(me, you, spell, 300, 8, 11) then
        return
    end

    if you:state() == STATE_GHOST then
        you:state(STATE_NORMAL)
        you:hp(math.max(30, you:hp()))
    end
end