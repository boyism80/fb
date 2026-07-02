-- item: 달콤새콤구이

-- 달콤새콤구이 사용 스크립트
function ON_ACTIVATED_3012(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
