function on_cast(me, you, spell)

end

function on_buff(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
end