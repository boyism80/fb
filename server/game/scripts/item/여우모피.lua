-- 여우모피 사용 스크립트
function ON_ACTIVATED_3022(me, item)
    me:hp(math.min(me:hp()+18, me:maxhp()))
end