-- 백두웅담 사용 스크립트
function ON_ACTIVE_3071(me, item)
    me:hp(math.min(me:hp()+650, me:maxhp()))
end