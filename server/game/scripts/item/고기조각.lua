-- 고기조각 사용 스크립트
function ON_ACTIVATED_3015(me, item)
    me:hp(math.min(me:hp()+8, me:maxhp()))
end