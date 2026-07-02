-- item: 궁극의도시락

-- 궁극의도시락 사용 스크립트
function ON_ACTIVATED_3082(me, item)
    me:hp(math.min(me:hp()+600, me:maxhp()))
end
