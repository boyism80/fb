-- item: 2차도시락

-- 2차도시락 사용 스크립트
function ON_ACTIVATED_3088(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end
