function NPC_405(me, npc)
    local messages = {
        "암흑왕인가 하는 굉장히 무서운 괴물이 어디에 살고 있다던데.. 어떻게 생긴걸까?",
        "선비족 두령은 정말 선비같이 생겼어. 그래서 선비족인걸까?",
        "얼마전에 현도성에서 길을 잃었는데, 살쾡이들이 갑자기 공격을 하더라고. 아아, 정말 무서웠어..",
        "중국의 서쪽에도 사람이 살고 있을까?",
        "북방지역에 지름길이 있다는거 알고 있나? 전에 우연히 발견했지.",
    }
    local msg = messages[math.random(1, #messages)]
    local button = me:dialog(npc, msg, false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
