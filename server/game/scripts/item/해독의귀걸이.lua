-- 해독의귀걸이 사용 스크립트
function ON_ACTIVATED_2129(me, item)

end

-- 해독의귀걸이 해제 스크립트
function ON_INACTIVE_2129(me, item)

end

-- 해독의귀걸이 지속 스크립트
function ON_CONCAST_2129(me, item)
    if me:isbuff('중독') then
        me:unbuff('중독')
    end
end