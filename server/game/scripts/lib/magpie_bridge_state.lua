-- Shared Magpie Bridge (오작교) ceremony state via server property
-- (package.loaded is per lua root / thread; timer re-opens the script).

local PREFIX = 'magpie_bridge.'

local DEFAULTS = {
    check = 0,
    gyeonu_failed = 0,
    jiknyeo_failed = 0,
    gyeonu_talk = 0,
    jiknyeo_talk = 0,
    couple_tick = 0,
    couple_buff_index = 1,
    countdown_active = false,
    chilsung_countdown = false,
    reunited = false,
    warped = false,
    baek_running = false,
    baek_started = false,
    baek_done = false,
    meet_running = false,
    tick_busy = false,
    settle_key = '',
    clock_stamp = 0,
    bridge_dirty = false,
    settle_mobs_cleared = false,
    bridge_build_active = false,
    bridge_build_start_sod = 0,
}

local M = {}

function M.get(key)
    local v = property(PREFIX .. key)
    if v == nil then
        return DEFAULTS[key]
    end
    return v
end

function M.set(key, value)
    property(PREFIX .. key, value)
end

function M.bump(key, delta)
    local v = M.get(key) + (delta or 1)
    M.set(key, v)
    return v
end

function M.reset()
    for key, default in pairs(DEFAULTS) do
        property(PREFIX .. key, default)
    end
end

return M
