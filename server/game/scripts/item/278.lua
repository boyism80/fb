-- item: 용겸제삼봉

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    ON_ATTACK = function(me, item)
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
        front:buff('용의제삼노', 6, me)
    end
}
