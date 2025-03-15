-- 대장군의일기2 사용 스크립트
function ON_ACTIVE_10202(me, item)
    local model = item:model()
    local button = nil
::ON_ACTIVE_10202_001::
    button = model:dialog(me, '-궐기한지 27일째\n천황이 나의 반역 사실을 눈치채고 대비하기 전에\n최대한 빨리 다른 영지를 무력화시킨다는 작전은 주효했다.\n대부분의 영지는 무력화시키거나 우리 편으로 만들었고, 남은 영지들은 뿔뿔이 흩어져 있어 천황이 남은 영지를 규합하여 우리 군에 대항하기도 힘들 것이다.', false, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end
::ON_ACTIVE_10202_002::
    button = model:dialog(me, '이제 천하통일의 그날까지 얼마 남지 않았다. 하지만 어쩐지 불안한 것은 왜일까? 좋지 않은 예감이 든다...', true, false)
    if button == DIALOG_RESULT_PREV then
        goto ON_ACTIVE_10202_001
    end
end