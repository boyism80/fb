local resp     = require("integration.response")
local protocol = require("integration.protocol")

local M = {}

function M.run_chat_step(bot, command, pass_prefix, fail_prefix)
    log("debug", string.format(
        "run_chat_step: bot=%s command=%q pass=%q fail=%q",
        bot:name(),
        command,
        pass_prefix,
        fail_prefix))

    local packet = bot:request(
        resp.chat,
        protocol.chat(false, command),
        function(p)
            local text = p.text or ""
            local pass_match = string.find(text, pass_prefix, 1, true) ~= nil
            local fail_match = string.find(text, fail_prefix, 1, true) ~= nil
            log("debug", string.format(
                "chat validator: text=%q pass_match=%s fail_match=%s",
                text,
                tostring(pass_match),
                tostring(fail_match)))
            return pass_match or fail_match
        end)

    local pass = string.find(packet.text or "", pass_prefix, 1, true) ~= nil
    log("debug", string.format(
        "run_chat_step result: bot=%s pass=%s text=%q",
        bot:name(),
        tostring(pass),
        packet.text or ""))
    return pass, packet.text
end

function M.run_script(bot, script_file, ...)
    local step = select(2, ...)
    if step == nil or step == "" then
        error("run_script: missing step name")
    end

    local parts = { "/스크립트", script_file, ... }
    local command = table.concat(parts, " ")
    local pass_prefix = "CL:PASS:" .. step
    local fail_prefix = "CL:FAIL:" .. step
    return M.run_chat_step(bot, command, pass_prefix, fail_prefix)
end

function M.run_script_with_prefix(bot, script_file, pass_prefix, fail_prefix, ...)
    local parts = { "/스크립트", script_file, ... }
    local command = table.concat(parts, " ")
    return M.run_chat_step(bot, command, pass_prefix, fail_prefix)
end

return M
