local resp     = require("integration.response")
local protocol = require("integration.protocol")

local M = {}

function M.form(ctx)
    local caster = ctx:bot(0)
    for i = 1, ctx:bot_count() - 1 do
        local target = ctx:bot(i)
        local r = caster:request(
            resp.message,
            protocol.group(target:name()),
            function(packet)
                return packet.type == "STATE"
                    and packet.text:find("님 그룹에 참여") ~= nil
            end)
        log("debug", r.text)
    end
end

function M.cleanup(ctx)
    for i = 0, ctx:bot_count() - 1 do
        local bot = ctx:bot(i)
        for _ = 1, 2 do
            bot:request(
                resp.message,
                protocol.update_option("GROUP", false),
                function(packet)
                    return packet.type == "STATE"
                        and packet.text:find("그룹허가") ~= nil
                end)
        end
    end
end

return M
