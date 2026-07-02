-- item: 산삼

-- 산삼 사용 스크립트
function ON_ACTIVATED_3035(me, item)
    me:hp(math.min(me:hp()+12, me:maxhp()))
end
