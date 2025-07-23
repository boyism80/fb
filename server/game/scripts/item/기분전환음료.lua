-- 기분전환음료 사용 스크립트
function ON_ACTIVE_3083(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end