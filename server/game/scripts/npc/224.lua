-- npc: 청룡성문지기
local castle_lib = require('lib.castle')

local TOTEM = '청룡'
local DIVINE_BEAST_ID = DIVINE_BEAST.AZURE_DRAGON

return {
    on_click = function(me, npc)
        local selected, button = me:pursuit(npc, '안녕하세요. 어떻게 오셨나요?', { TOTEM .. '성 입장' })
        if button == DIALOG_RESULT.QUIT or selected ~= 1 then
            return
        end

        castle_lib.enter_castle(me, npc, TOTEM, DIVINE_BEAST_ID)
    end
}
