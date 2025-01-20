function on_cast(me, spell)
    local damage = math.floor(me:hp()*0.75 + me:mp()*1.5)
    local mp = math.max(30, me:mp() // 2)
    local sound = 500
    local effect = 161

    if not spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 6, 5), spell, damage, mp, sound, effect) then
    	return
    end
    
    me:hp(me:hp() // 2)
end