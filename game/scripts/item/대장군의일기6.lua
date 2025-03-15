-- 대장군의일기6 사용 스크립트
function ON_ACTIVE_10206(me, item)
    local model = item:model()
    local button = nil
    
::ON_ACTIVE_10206_001::
    button = model:dialog(me, '-궐기한지...??일째\n오늘은 146명. 아니, 148명이였던가?\n악마와 약속한 숫자까지 얼마 남지 않았다.\n사실 그런 건 아무래도 좋다.\n어떻게 하면 더 많은 피를 볼 수 있을까? 어떻게 하면...?\n그래, 더 커다란 전쟁을 일으키는 것은 어떨까? 악마와 계약한 대로 영원한 생명을 얻게 된다면 더 큰 전쟁을 일으켜 보는 것도 좋겠다...', false, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end
::ON_ACTIVE_10206_002::
    button = model:dialog(me, '아...\n그러고 보니... 뭐였더라.\n내가 왜 반란군을 일으켰는지 생각이 잘 나지 않는다.\n고통받는 사람들을... 어떻게 한다는 생각이였던거 같은데...\n역시 기억이 잘 나지 않는다. 별로 생각하고 싶지도 않고...\n뭐, 아무려면 어떠랴, 더 많은 피를 볼 수 있다면야...흐흐흐...', true, false)
    if button == DIALOG_RESULT_PREV then
        goto ON_ACTIVE_10206_001
    end
end