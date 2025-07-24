-- 녹용 사용 스크립트
function ON_ACTIVE_3031(me, item)
    me:hp(math.min(me:hp()+148, me:maxhp()))
end