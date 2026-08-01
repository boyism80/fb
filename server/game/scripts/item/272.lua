-- item: 용랑제육봉

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    on_attack = function(me, item)
        math.randomseed(seed())
        if math.random() > 0.25 then
            return false
        end
        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            return
        end
        front:sound(40)
        front:effect(53)
        front:buff('용의제육주', 21, me)
    end
}
