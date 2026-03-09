-- 보라시약 사용 스크립트
function ON_ACTIVATED_3093(me, item)
    me:hp(math.min(me:hp()+200, me:maxhp()))
end