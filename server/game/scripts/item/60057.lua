-- item: 국화주
-- Same effect as 팔십세주 (MP +150).

return {
    on_activated = function(me, item)
        me:mp(math.min(me:mp() + 150, me:maxmp()))
    end,
}
