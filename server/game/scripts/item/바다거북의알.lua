-- 바다거북의알 사용 스크립트
function ON_ACTIVE_3073(me, item)
    me:hp(math.min(me:hp()+500, me:maxhp()))
end