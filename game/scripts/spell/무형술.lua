function on_cast(me, you, spell)

end

function on_uncast(me, spell)
    me:buff_phydef(me:buff_phydef() - 40)
    unbuff(me, spell)
end