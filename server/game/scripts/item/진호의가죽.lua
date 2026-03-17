-- 진호의가죽 사용 스크립트
function ON_ACTIVATED_3036(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end