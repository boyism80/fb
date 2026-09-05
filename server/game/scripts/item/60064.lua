-- item: 설화의이슬
-- 섣달 reward drink: MP +500 per use (durability 300).

return {
    on_activated = function(me, item)
        me:mp(math.min(me:mp() + 500, me:maxmp()))
    end,
}
