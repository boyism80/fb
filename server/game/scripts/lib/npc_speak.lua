-- NPC chat helpers for timed ceremony lines.

local M = {}

function M.by_id(map, npc_id, text, fallback_name)
    if map ~= nil then
        for _, npc in pairs(map:objects(OBJECT_TYPE.NPC)) do
            if npc:model():id() == npc_id then
                npc:chat(text)
                return
            end
        end
    end
    if fallback_name ~= nil then
        broadcast(fallback_name .. ': ' .. text, MESSAGE_TYPE.NOTIFY)
    end
end

-- Speak lines with sleep between them. still_active() may abort mid-sequence.
function M.lines(map, npc_id, lines, interval_ms, still_active, fallback_name)
    for i, line in ipairs(lines) do
        if still_active ~= nil and not still_active() then
            break
        end
        M.by_id(map, npc_id, line, fallback_name)
        if i < #lines then
            sleep(interval_ms)
        end
    end
end

return M
