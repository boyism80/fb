function ON_ACTIVATED_9050(me, item)
    local front = me:front(~OBJECT_TYPE.ITEM)
    if front == nil then
        return
    end

    me:cast(front, '의태')
end

function ON_INACTIVE_9050(me, item)
end
