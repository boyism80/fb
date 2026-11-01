local M = {}

function M.reply(bot, response, packet, filter, timeout)
    local ok, result = pcall(bot.request, bot, response, packet, filter, timeout)
    if ok and result ~= false then
        return result
    end
    return nil
end

return M
