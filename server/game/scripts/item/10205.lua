-- item: 대장군의일기5
-- 대장군의일기5 사용 스크립트
function ON_ACTIVATED_10205(me, item)
    local model = item:model()
    local button = nil

::ON_ACTIVATED_10205_001::
    button = me:dialog(model, '-궐기한지 35일째\n이상한 꿈을 꾸었다.\n기괴한 생김새의 악마가,\n내가 죄없는 목숨을 많이 베면 벨 수록 더 강대한 힘을 얻을 수 있도록 해 주고,\n10만명의 목숨을 베는 날, 나에게 영원한 생명을 주겠다는 이야기를 했다.\n그리고 계약의 증거로, 다음 전투에서 우리 군이 반드시 이기게 해 줄 것이라고 했다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
::ON_ACTIVATED_10205_002::
    button = me:dialog(model, '꿈이여서 그랬는지, 나는 홀린 듯이 그러겠다고 말했고, 그 순간 꿈에서 깨어났다.\n내가 본 것은 정말 꿈이었을까?\n모든 것은 내일 있을 전투가 증명해 줄 것이다. 꿈에서 한 악마와의 계약이 만약 진짜였다고 해도,\n그것이 고통받는 사람이 없는 세상을 만들기 위해서라면...', true, false)
    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10205_001
    end
end
