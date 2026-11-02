local M = {}

function M.strip(bot, parts)
    local wanted = {}
    for _, part in ipairs(parts) do
        wanted[part] = true
    end
    for _, slot in ipairs(brain.new():worn(bot)) do
        if wanted[slot.type] and #slot.names > 0 then
            bot:unequip(slot.type)
        end
    end
end

return M
