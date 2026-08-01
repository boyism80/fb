-- npc: 흉노토벌장군
local WARP_OPTIONS = {
    { "흉노족6", "흉노족6-2", 38, 28 },
    { "흉노족7", "흉노족7-2", 30, 38 },
    { "흉노족8", "흉노족8-2", 30, 38 },
    { "흉노족9", "흉노족9-2", 28, 2 },
    { "흉노족10", "흉노족10-2", 28, 2 },
}

return {
    on_click = function(me, npc)
    ::NPC_462_0000::
        local button = me:dialog(npc, "황제께서 흉노를 토벌하기 위해 그대들을 파견했건만, 여전히 모든 흉노를 토벌하기에는 역부족이었다네.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

    ::NPC_462_0001::
        button = me:dialog(npc, "여전히 저 깊은 골짜기에는 흉노족의 놈들이 장안성을 침공하기 위해 세를 기르고 있다네.", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_462_0000
        end

        local sel, list_btn = me:list(npc, "어때, 저 깊은 곳에 있는 흉노떼들을 무찌르기 위해 입장 하겠는가? 저 안에서 아이템을 잃어버릴수도 있다네.", {
            "흉노족6",
            "흉노족7",
            "흉노족8",
            "흉노족9",
            "흉노족10",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel < 1 or sel > 5 then
            return
        end

        local opt = WARP_OPTIONS[sel]
        local map = name2map(opt[2])
        if map == nil then
            return
        end
        me:map(map, opt[3], opt[4])
    end
}
