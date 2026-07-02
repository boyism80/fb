-- item: 생장촉진통구이

-- 생장촉진통구이 사용 스크립트
function ON_ACTIVATED_3008(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
