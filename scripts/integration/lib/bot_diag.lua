-- Shared bot state dump for diagnosing integration test failures.
local M = {}

local function map_label(map_id)
    if map_id == nil then
        return "nil"
    end
    local model = id2map(map_id)
    if model == nil then
        return string.format("id=%s(unknown)", tostring(map_id))
    end
    return string.format("id=%s name=%s", tostring(map_id), tostring(model:name()))
end

function M.dump(bot, label)
    label = label or "bot"
    if bot == nil then
        log("fatal", string.format("[bot_diag] %s bot=nil", label))
        return
    end

    local ok, err = pcall(function()
        local pos = bot:position()
        local map_id = bot:map()
        log("fatal", string.format(
            "[bot_diag] %s name=%s oid=%s map={%s} pos=(%s,%s) hp=%s/%s mp=%s/%s level=%s state=%s",
            label,
            tostring(bot:name()),
            tostring(bot:oid()),
            map_label(map_id),
            tostring(pos and pos[1]),
            tostring(pos and pos[2]),
            tostring(bot:hp()),
            tostring(bot:base_hp()),
            tostring(bot:mp()),
            tostring(bot:base_mp()),
            tostring(bot:level()),
            tostring(bot:state())))
    end)
    if ok == false then
        log("fatal", string.format("[bot_diag] %s dump failed: %s", label, tostring(err)))
    end
end

function M.dump_many(bots, label)
    label = label or "bots"
    if bots == nil then
        log("fatal", string.format("[bot_diag] %s bots=nil", label))
        return
    end
    for i, bot in ipairs(bots) do
        M.dump(bot, string.format("%s[%d]", label, i))
    end
end

return M
