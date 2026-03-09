-- 바나나 사용 스크립트
function ON_ACTIVATED_3074(me, item)
    me:hp(math.min(me:hp()+30, me:maxhp()))
end