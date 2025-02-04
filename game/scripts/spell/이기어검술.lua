function on_cast(me, you, spell)

end

function on_uncast(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
    unbuff(me, spell)
end