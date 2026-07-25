-- npc: 장안경비대장

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "폐하께 전할 말씀은 나를 통해서 하시오!", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
