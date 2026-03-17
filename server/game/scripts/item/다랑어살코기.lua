-- 다랑어살코기 사용 스크립트
function ON_ACTIVATED_3079(me, item)
    me:hp(math.min(me:hp()+30, me:maxhp()))
end