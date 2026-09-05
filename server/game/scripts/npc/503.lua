-- npc: 보름달

return {
    on_click = function(me, npc)
        me:dialog(npc, " 밝은 달이 떠 있다. 어쩐지 움직일 것 같다.", { prev = false, next = false })
    end
}
