-- item: 보무의목걸이

-- 보무의목걸이 사용 스크립트
function ON_ACTIVATED_2135(me, item)

end

-- 보무의목걸이 해제 스크립트
function ON_DEACTIVATED_2135(me, item)

end

-- 보무의목걸이 지속 스크립트
function ON_CONCAST_2135(me, item)
    me:cast('자동무장')
    me:cast('자동보호')
end
