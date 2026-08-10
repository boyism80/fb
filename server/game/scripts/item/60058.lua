-- item: 약신의물병
-- Same effect as 팔십세주 (MP +150). Monthly collectible for 중양절.

return {
    on_activated = function(me, item)
        me:mp(math.min(me:mp() + 150, me:maxmp()))
    end,
}
