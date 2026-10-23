local festival = require('lib.festival')

local M = {
    MACHINE = 'magpie_bridge',
    MAP_ID = 10957,
    CHILSUNG_MAP_ID = 10954,
    BRIDGE_Y = 13,
    BRIDGE_X1 = 24,
    BRIDGE_X2 = 81,
    BRIDGE_BUILD_SECONDS = 300,
    BRIDGE_HOURS = {
        [0] = true,
        [6] = true,
        [12] = true,
        [18] = true,
    },
}

function M.seconds_of_day(t)
    if t == nil then
        return 0
    end
    return ((t.hour or 0) * 3600) + ((t.minute or 0) * 60) + (t.second or 0)
end

function M.bridge_build_remaining(t)
    local sm = state_machine(M.MACHINE)
    if sm == nil then
        return nil
    end
    local start_sod = sm:get('bridge_build_start_sod')
    if start_sod == nil then
        return nil
    end
    t = t or datetime()
    if t == nil then
        return nil
    end
    local elapsed = M.seconds_of_day(t) - start_sod
    if elapsed < 0 then
        elapsed = elapsed + 86400
    end
    return math.max(0, M.BRIDGE_BUILD_SECONDS - elapsed)
end

function M.round_key()
    local sm = state_machine(M.MACHINE)
    if sm == nil then
        return nil
    end
    local key = sm:get('settle_key')
    if key == nil or key == '' then
        return nil
    end
    return key
end

function M.sync_timer(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('칠석') then
        return
    end
    local t = datetime()
    if t == nil or M.BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end
    local rem = M.bridge_build_remaining(t)
    if rem == nil or rem <= 0 then
        return
    end
    me:timer(rem, TIMER_TYPE.DECREASE)
end

function M.on_bridge_enter(me)
    M.sync_timer(me)
    local sm = state_machine(M.MACHINE)
    if sm ~= nil then
        sm:call('on_bridge_enter')
    end
end

function M.on_step(me)
    if me == nil then
        return
    end
    if not festival.is('칠석') then
        return
    end

    local t = datetime()
    if t == nil or M.BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end
    local rem = M.bridge_build_remaining(t)
    if rem == nil or rem <= 0 then
        return
    end

    local map = me:map()
    if map == nil or map:model():id() ~= M.MAP_ID then
        return
    end

    local x, y = me:position()
    if y ~= M.BRIDGE_Y or x < M.BRIDGE_X1 or x > M.BRIDGE_X2 then
        return
    end

    if map:at(x, y + 2) == nil then
        me:map(map, x, y + 2)
    end
end

return M
