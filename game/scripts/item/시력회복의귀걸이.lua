function on_active(me, item)

end

function on_inactive(me, item)

end

function on_concast(me, item)
    if me:isbuff('절망') then
        me:unbuff('절망')
    end
end