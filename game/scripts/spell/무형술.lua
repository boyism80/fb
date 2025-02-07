function on_cast(me, you, spell)

end

function on_buff(me, spell)
    mee:buff_phydef(me:buff_phydef() - 40)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() - 40)
end