-- item: 끔찍한요리

-- 끔찍한요리 사용 스크립트
function ON_ACTIVATED_3085(me, item)
    me:hp(math.min(me:hp()+5, me:maxhp()))
end
