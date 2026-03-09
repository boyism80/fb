-- 감자 사용 스크립트
function ON_ACTIVATED_3044(me, item)
    me:hp(math.min(me:hp()+50, me:maxhp()))
end