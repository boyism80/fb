-- 참수박 사용 스크립트
function ON_ACTIVATED_3033(me, item)
    me:hp(math.min(me:hp()+312, me:maxhp()))
end