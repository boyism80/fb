-- npc: 백호성문지기
local castle_lib = require('lib.castle')

local TOTEM = '백호'
local DIVINE_BEAST_ID = DIVINE_BEAST.WHITE_TIGER

return {
    on_click = function(me, npc)
        local OPT_ENTER = TOTEM .. '성 입장'
        local selected, button = me:pursuit(npc, '안녕하세요. 어떻게 오셨나요?', { OPT_ENTER })
        if button == DIALOG_RESULT.QUIT or selected ~= OPT_ENTER then
            return
        end

        castle_lib.enter_castle(me, npc, TOTEM, DIVINE_BEAST_ID)
    end
}
