-- item: 복어의심장

-- 복어의심장 사용 스크립트
function ON_ACTIVATED_3072(me, item)
    me:hp(math.min(me:hp()+150, me:maxhp()))
end
