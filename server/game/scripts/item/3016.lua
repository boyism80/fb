-- item: 고기

-- 고기 사용 스크립트
function ON_ACTIVATED_3016(me, item)
    me:hp(math.min(me:hp()+10, me:maxhp()))
end
