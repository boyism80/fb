-- item: 노란시약

-- 노란시약 사용 스크립트
function ON_ACTIVATED_3091(me, item)
    me:hp(math.min(me:hp()+50, me:maxhp()))
end
