-- 기분전환반찬 사용 스크립트
function ON_ACTIVATED_3084(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end