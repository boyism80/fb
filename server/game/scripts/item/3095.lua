-- item: 빨간시약

-- 빨간시약 사용 스크립트
function ON_ACTIVATED_3095(me, item)
    local value = me:maxhp() // 3
    me:hp(math.min(me:hp()+value, me:maxhp()))
end
