-- 파란시약 사용 스크립트
function ON_ACTIVATED_3092(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end