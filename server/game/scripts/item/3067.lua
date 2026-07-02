-- item: 식용벌레

-- 식용벌레 사용 스크립트
function ON_ACTIVATED_3067(me, item)
    me:hp(math.min(me:hp()+30, me:maxhp()))
end
