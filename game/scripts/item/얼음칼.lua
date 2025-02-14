function on_attack(me, item)
    math.randomseed(seed())
    if math.random() > 0.3 then
        return false
    end

    local front = me:front(OBJECT_TYPE_LIFE)
    if front == nil then
        return
    end

	if front:isbuff(table.unpack(relative_buff_name('마비'))) then
    	return
    end

    front:sound(70)
    front:effect(1)
    front:buff('마비', 20, me)
end

function on_active(me, item)

end

function on_inactive(me, item)

end