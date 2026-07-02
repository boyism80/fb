-- item: 늑대고기

-- 늑대고기 사용 스크립트
function ON_ACTIVATED_3081(me, item)
    me:hp(math.min(me:hp()+30, me:maxhp()))
end
