-- npc: 유령화화
function NPC_380(me, npc)
    ::NPC_380_0001::
    local btn = me:dialog(npc, "멀리 어렴풋한 무언가가 보인다.\n\n절벽에 가까이 서서 보면 멀리 소녀가 서있는 것 같다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_380_0002::
    btn = me:dialog(npc, "먼곳을 바라보고 있는 소녀의 얼굴은 어쩐지 슬퍼보인다.\n\n불러보고, 손짓을 해도 소녀는 아무런 대답이 없다.", true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_380_0001
    end
end
