-- item: 3차도시락

-- 3차도시락 사용 스크립트
function ON_ACTIVATED_3089(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end
