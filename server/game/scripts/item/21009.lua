-- item: 밀전병

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp() + 200, me:maxhp()))
    end,
}