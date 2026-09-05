-- item: 소금
-- Kimjang material; usable: HP +1 per consume.

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp() + 1, me:maxhp()))
    end,
}
