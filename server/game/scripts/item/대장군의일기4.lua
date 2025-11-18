-- 대장군의일기4 사용 스크립트
function ON_ACTIVE_10204(me, item)
    local model = item:model()
    local button = nil

::ON_ACTIVE_10204_001::
    button = me:dialog(model, '-궐기한지 34일째\n절망적인 상황이다.\n3일 전의 전투에서 대패한 결과, 병사는 모두 뿔뿔이 흩어졌고, 남은 병력은 원래의 절반도 되지 않는다.\n고통받는 이가 없는 세상은 커녕,\n나는 지금 우리 병사들 하나하나에게 더할 나위 없는 고통을 주고 있는 것은 아닌가....\n오늘만 해도 도망치는 병사를 셋이나 베었다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
::ON_ACTIVE_10204_002::
    button = me:dialog(model, '그들을 이해할 수 없는 것은 아니지만, 남은 병사의 사기를 위해 벨 수밖에 없었다.\n내 손으로 벤 병사 중 한 명의 짐 속에서 이상한 물건을 발견했다.\n사악한 기운이 느껴지는 부적이다.\n평소 같았으면 이런 사악한 물건은 당장 불태워 없앴겠지만...\n지금은 이런 하찮은 부적에라도 의지하고 싶다....', true, false)
    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVE_10204_001
    end
end