-- item: 야자수액

-- 야자수액 사용 스크립트
function ON_ACTIVATED_3046(me, item)
    me:hp(math.min(me:hp()+5, me:maxhp()))
end
