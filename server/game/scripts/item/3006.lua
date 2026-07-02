-- item: 생명보존음료

-- 생명보존음료 사용 스크립트
function ON_ACTIVATED_3006(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
