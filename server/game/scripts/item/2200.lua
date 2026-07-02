-- item: 동동주

-- 동동주 사용 스크립트
function ON_ACTIVATED_2200(me, item)
    me:hp(math.max(1, me:hp()-1), false)
    me:mp(math.min(me:mp()+30, me:maxmp()), false)
    me:update()
end
