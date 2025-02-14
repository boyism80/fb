function on_active(me, item)

end

function on_inactive(me, item)

end

function on_concast(me, item)
    if me:isbuff('중독') then
        me:unbuff('중독')
    end
end