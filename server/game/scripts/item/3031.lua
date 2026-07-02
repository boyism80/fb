-- item: 녹용

-- 녹용 사용 스크립트
function ON_ACTIVATED_3031(me, item)
    me:hp(math.min(me:hp()+148, me:maxhp()))
end
