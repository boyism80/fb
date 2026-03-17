-- 대장군의일기통합본 사용 스크립트
function ON_ACTIVATED_10208(me, item)
    local model = item:model()
    local button = nil
::ON_ACTIVATED_10208_001::
    button = me:dialog(model, '-궐기 전야\n이미 천왕은 통치자로서의 분별력을 잃은 지 오래, 사리사욕을 채우기 급급하여 백성들의 피를 빠는 폭군이 되고 말았다.\n다른 영지를 통치하는 영주들도 백성들이 마음놓고 살 수 있도록 영지를 잘 다스리기는 커녕, 오히려 서로 영지를 빼앗으려는 크고 작은 전쟁을 일으켜 천하의 백성들을 슬프게 하고 있다. 천하는 이미 전쟁으로 가득한, 이른바 전국시대에 접어들었다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

::ON_ACTIVATED_10208_002::
    button = me:dialog(model, '이런 시대에 대장군인 내가 할 수 있는 일은 단 하나, 난세를 평정해 천하통일을 이룩하는 것 뿐.\n그렇게 하면 더 이상의 전쟁의 고통에 괴로워하는 이도 없을터', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_001
    end
::ON_ACTIVATED_10208_003::
    button = me:dialog(model, '-궐기한지 27일째\n천황이 나의 반역 사실을 눈치채고 대비하기 전에\n최대한 빨리 다른 영지를 무력화시킨다는 작전은 주효했다.\n대부분의 영지는 무력화시키거나 우리 편으로 만들었고, 남은 영지들은 뿔뿔이 흩어져 있어 천황이 남은 영지를 규합하여 우리 군에 대항하기도 힘들 것이다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_002
    end
::ON_ACTIVATED_10208_004::
    button = me:dialog(model, '이제 천하통일의 그날까지 얼마 남지 않았다. 하지만 어쩐지 불안한 것은 왜일까? 좋지 않은 예감이 든다...', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_003
    end
::ON_ACTIVATED_10208_005::
    button = me:dialog(model, '-궐기한지 31일째\n예감은 적중했다.\n천황은 남은 영지에 밀사를 보내 연합군을 조직, 우리를 사방에서 포위하고 공격해왔다.\n천황의 지독한 폭정에 모두 반발하고 있었으니, 천황의 명을 따르지 않을 것이라고 생각했건만. 분명 많은 부와 더 좋은 지위를 미끼로 유혹했겠지...', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_004
    end
::ON_ACTIVATED_10208_006::
    button = me:dialog(model, '일기를 쓰고 있을 시간이 없다.\n오늘 밤에 있을 전투가 우리 군의 미래를 결정할 것이다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_005
    end
::ON_ACTIVATED_10208_007::
    button = me:dialog(model, '-궐기한지 34일째\n절망적인 상황이다.\n3일 전의 전투에서 대패한 결과, 병사는 모두 뿔뿔이 흩어졌고, 남은 병력은 원래의 절반도 되지 않는다.\n고통받는 이가 없는 세상은 커녕,\n나는 지금 우리 병사들 하나하나에게 더할 나위 없는 고통을 주고 있는 것은 아닌가....\n오늘만 해도 도망치는 병사를 셋이나 베었다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_006
    end
::ON_ACTIVATED_10208_008::
    button = me:dialog(model, '그들을 이해할 수 없는 것은 아니지만, 남은 병사의 사기를 위해 벨 수밖에 없었다.\n내 손으로 벤 병사 중 한 명의 짐 속에서 이상한 물건을 발견했다.\n사악한 기운이 느껴지는 부적이다.\n평소 같았으면 이런 사악한 물건은 당장 불태워 없앴겠지만...\n지금은 이런 하찮은 부적에라도 의지하고 싶다....', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_007
    end
::ON_ACTIVATED_10208_009::
    button = me:dialog(model, '-궐기한지 35일째\n이상한 꿈을 꾸었다.\n기괴한 생김새의 악마가,\n내가 죄없는 목숨을 많이 베면 벨 수록 더 강대한 힘을 얻을 수 있도록 해 주고,\n10만명의 목숨을 베는 날, 나에게 영원한 생명을 주겠다는 이야기를 했다.\n그리고 계약의 증거로, 다음 전투에서 우리 군이 반드시 이기게 해 줄 것이라고 했다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_008
    end
::ON_ACTIVATED_10208_010::
    button = me:dialog(model, '꿈이여서 그랬는지, 나는 홀린 듯이 그러겠다고 말했고, 그 순간 꿈에서 깨어났다.\n내가 본 것은 정말 꿈이었을까?\n모든 것은 내일 있을 전투가 증명해 줄 것이다. 꿈에서 한 악마와의 계약이 만약 진짜였다고 해도,\n그것이 고통받는 사람이 없는 세상을 만들기 위해서라면...', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_009
    end
::ON_ACTIVATED_10208_011::
    button = me:dialog(model, '-궐기한지...??일째\n오늘은 146명. 아니, 148명이였던가?\n악마와 약속한 숫자까지 얼마 남지 않았다.\n사실 그런 건 아무래도 좋다.\n어떻게 하면 더 많은 피를 볼 수 있을까? 어떻게 하면...?\n그래, 더 커다란 전쟁을 일으키는 것은 어떨까? 악마와 계약한 대로 영원한 생명을 얻게 된다면 더 큰 전쟁을 일으켜 보는 것도 좋겠다...', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_010
    end
::ON_ACTIVATED_10208_012::
    button = me:dialog(model, '아...\n그러고 보니... 뭐였더라.\n내가 왜 반란군을 일으켰는지 생각이 잘 나지 않는다.\n고통받는 사람들을... 어떻게 한다는 생각이였던거 같은데...\n역시 기억이 잘 나지 않는다. 별로 생각하고 싶지도 않고...\n뭐, 아무려면 어떠랴, 더 많은 피를 볼 수 있다면야...흐흐흐...', true, false)
    if button == DIALOG_RESULT.PREV then
        goto ON_ACTIVATED_10208_011
    end
end