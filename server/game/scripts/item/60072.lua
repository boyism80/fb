-- item: 배추김치
-- Kimjang festival kimchi: HP +1000 per use (durability 40).

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp() + 1000, me:maxhp()))
    end,
}
