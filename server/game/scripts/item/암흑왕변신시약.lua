-- 암흑왕변신시약 사용 스크립트
function ON_ACTIVE_3105(me, item)
    local look = name2mob('암흑왕'):look()
    me:mimic({ disguise = look })
    me:effect(3)
    me:sound(25)
end