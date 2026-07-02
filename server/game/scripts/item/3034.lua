-- item: 인삼

-- 인삼 사용 스크립트
function ON_ACTIVATED_3034(me, item)
    me:hp(math.min(me:hp()+12, me:maxhp()))
end
