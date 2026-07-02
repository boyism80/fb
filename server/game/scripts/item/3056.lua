-- item: 파수다지오

-- 파수다지오 사용 스크립트
function ON_ACTIVATED_3056(me, item)
    me:hp(math.min(me:hp()+10, me:maxhp()))
end
