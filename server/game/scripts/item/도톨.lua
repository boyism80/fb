-- 도톨 사용 스크립트
function ON_ACTIVE_9(me, item)
    local q = me:quest(QUEST_NAKRANG2)
    if q and not q:completed() and q:step() == 1 and q:progress() < 2 then
        q:progress(q:progress() + 1)
    end
    me:hp(math.min(me:hp() + 8, me:maxhp()))
end