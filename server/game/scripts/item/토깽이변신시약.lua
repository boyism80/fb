-- 토깽이변신시약 사용 스크립트
function ON_ACTIVE_3104(me, item)
    local look = name2mob('토깽이'):look()
    me:mimic({ disguise = look })
    me:effect(3)
    me:sound(25)
end