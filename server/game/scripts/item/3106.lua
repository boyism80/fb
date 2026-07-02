-- item: 청의태자변신시약

-- 청의태자변신시약 사용 스크립트
function ON_ACTIVATED_3106(me, item)
    local look = name2mob('청의태자'):look()
    me:mimic({ disguise = look })
    me:effect(3)
    me:sound(25)
end
