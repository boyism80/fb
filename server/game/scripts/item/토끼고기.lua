-- 토끼고기 사용 스크립트
function ON_ACTIVATED_3018(me, item)
    me:hp(math.min(me:hp()+48, me:maxhp()))
end