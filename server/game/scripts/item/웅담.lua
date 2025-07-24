-- 웅담 사용 스크립트
function ON_ACTIVE_3032(me, item)
    me:hp(math.min(me:hp()+500, me:maxhp()))
end