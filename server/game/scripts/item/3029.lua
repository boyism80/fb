-- item: 말고기

-- 말고기 사용 스크립트
function ON_ACTIVATED_3029(me, item)
    me:hp(math.min(me:hp()+108, me:maxhp()))
end
