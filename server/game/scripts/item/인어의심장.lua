-- 인어의심장 사용 스크립트
function ON_ACTIVE_3059(me, item)
    me:hp(math.min(me:hp()+100, me:maxhp()))
end