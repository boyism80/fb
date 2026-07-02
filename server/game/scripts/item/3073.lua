-- item: 바다거북의알

-- 바다거북의알 사용 스크립트
function ON_ACTIVATED_3073(me, item)
    me:hp(math.min(me:hp()+500, me:maxhp()))
end
