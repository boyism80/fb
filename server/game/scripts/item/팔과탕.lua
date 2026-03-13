-- 팔과탕 사용 스크립트
function ON_ACTIVATED_4026(me, item)
    me:hp(math.min(me:hp()+1000, me:maxhp()), false)
    me:mp(math.min(me:mp()+1000, me:maxmp()), false)
    me:update()
end