function on_active(me, item)

end

function on_inactive(me, item)

end

function on_concast(me, item)
    me:cast('자동무장')
    me:cast('자동보호')
end