-- 오도독망고과편 사용 스크립트
function ON_ACTIVE_3061(me, item)
    me:hp(math.min(me:hp()+800, me:maxhp()))
end