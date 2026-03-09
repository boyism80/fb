-- 박쥐고기 사용 스크립트
function ON_ACTIVATED_3070(me, item)
    me:hp(math.min(me:hp()+58, me:maxhp()))
end