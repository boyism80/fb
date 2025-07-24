-- 숲돼지고기 사용 스크립트
function ON_ACTIVE_3039(me, item)
    me:hp(math.min(me:hp()+148, me:maxhp()))
end