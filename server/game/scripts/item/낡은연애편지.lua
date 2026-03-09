-- 낡은연애편지 사용 스크립트
function ON_ACTIVATED_10249(me, item)
    local model = item:model()
    local button = nil
::ON_ACTIVATED_10249_001::
    button = me:dialog(model, '봉투를 열었더니, 안에서는 낡은 연애 편지 한장이 나왔다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
::ON_ACTIVATED_10249_002::
    button = me:dialog(model, '청하.. 이 겨울날에 나의 옆구리는 아주 따뜻하다오. 당신의 따뜻한 체온이 나를 감싸안아서 그런가보오. 나는 이 세상 끝나는 날까지 당신만 있으면 행복하다오. 청하! 나의 귀여운 꽃사슴. 사랑하오~!', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10249_001
    end
::ON_ACTIVATED_10249_003::
    button = me:dialog(model, '...라고 쓰여져 있군', true, false)
    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10249_002
    end
end