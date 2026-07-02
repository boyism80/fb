-- item: 새콤달콤구이

-- 새콤달콤구이 사용 스크립트
function ON_ACTIVATED_3005(me, item)
    me:hp(math.min(me:hp()+800, me:maxhp()))
end
