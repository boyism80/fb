-- item: 활력충전음료

-- 활력충전음료 사용 스크립트
function ON_ACTIVATED_3051(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
