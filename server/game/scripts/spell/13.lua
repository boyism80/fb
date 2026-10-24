-- spell: 자동투명
-- 자동투명 캐스팅
local spell = require('lib.spell')

-- 자동투명 버프 효과

-- 자동투명 버프 해제 효과

return {
    on_cast = function(me, sp)
        local buff_time = 180
        local state = me:state()
        if state == STATE.GHOST or state == STATE.RIDING then
            return false
        end

        if me:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
            return false
        end

        me:buff(sp, buff_time)
    end,

    on_buff = function(me, sp)
        local state = me:state()
        if state == STATE.GHOST or state == STATE.RIDING then
            return
        end

        me:state(STATE.HALF_CLOACK)
    end,

    on_unbuff = function(me, sp)
        spell.uncloak(me)
    end,

    -- on_concast = function(me, sp)
    -- end
}
