-- 말고기 사용 스크립트
function ON_ACTIVE_3029(me, item)
    me:hp(math.min(me:hp()+108, me:maxhp()))
end