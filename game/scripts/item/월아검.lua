function on_attack(me, item)
    math.randomseed(seed())
    if math.random() > 0.3 then
        return false
    end

    local front = me:front(OBJECT_TYPE_LIFE)
    if front == nil then
        return
    end

    me:cast(front, '월아일격')
end

function on_active(me, item)

end

function on_inactive(me, item)

end