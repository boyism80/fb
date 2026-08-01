-- item: 낡은봉투

local enum = require('lib.enum')

return {
    on_activated = function(me, item)
        local model = item:model()
        local selected = me:list(model, '봉투를 여시겠습니까?', {'열지 않겠어. 약속은 지켜야해.', '열어봐야지. 약속보다는 궁금함이 먼저야.'})
        if selected == nil then
            return
        end

        if selected == 1 then
            return
        end

        local code = me:exchange(
            { ['item'] = { ['낡은봉투'] = 1 } },
            { ['item'] = { ['낡은연애편지'] = 1 } }
        )
        if code ~= enum.exchange_result.OK then
            return
        end
        local added = me:item('낡은연애편지')
        me:active(added)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
