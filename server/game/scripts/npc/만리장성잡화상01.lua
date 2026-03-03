function NPC_394(me, npc)
::NPC_394_000::
    local sel, btn = me:list(npc, "안녕하신가.. 무슨일로 찾아왔나?", {"천패, 지패, 인패는 무엇인가요?", "뭐 재미있는 이야기 없습니까?"}, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        local button = me:dialog(npc, "여기 만리장성을 짓게한 진시황에 대해서 알고 있나? 진시황은 항상 불노불사의 약을 찾아다녔지. 영원히 살기 위해서 말일세.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "그러나 결국 찾지 못하고 죽어버렸지. 그러니 장안성에 진시황릉이 있지 않겠나? 죽어서 무슨 영화를 누리겠다고 자신의 무덤을 그렇게 크게 만들었는지.. 어쨌든 굉장히 불사의 삶에 대해 관심이 많았던 사람일세.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "알려지지 않은 이야기네만, 사실 진시황은 우주를 구성하는 세가지 요소인 '천' '지' '인'의 세가지를 상징하는 천지인의 증표를 얻었다고 하네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "진시황이 죽은걸 보면 그것을 가지고 있다고 해서 불노불사의 몸이 되는 것은 아니겠지만, 확실히 어떤 효과가 있긴 있는거 같네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "진시황 사후에 천지인의 증표는 셋으로 나눠져 여기저기로 흩어졌는데, 그것 때문에 우리 중국의 여러 성들에 이상한 현상이 일어났지.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "천구라는 하늘에서 산다는 괴물이 나타나기도하고, 땅속에 산다고 하는 후라는 괴물이 나타나질 않나, 심지어는 진시황릉속에선 강시가 나오기도 하지.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "천구는 하늘의 증표인 천패를, 후는 땅의 증표인 지패를, 그리고 강시는 인간의 증표인 인패를 가지고 있을 것이라 생각 되네.", false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "어쩌면 천구의 왕, 후의 왕, 강시의 왕은 패를 가지고 있을지도 모르지.", false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    elseif sel == 1 then
        local button = me:dialog(npc, "지금 우리 중국의 황제께서 흉노족때문에 고심하고 계신다고 하던데..", false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
    goto NPC_394_000
end
