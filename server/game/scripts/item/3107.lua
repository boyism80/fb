-- item: 잔치떡

-- 잔치떡 사용 스크립트
function ON_ACTIVATED_3107(me, item)
    me:hp(math.min(me:hp()+500, me:maxhp()))
end
