local lib         = require("integration.lib")
local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local HOLD_MS         = 2000
local MESSAGE_WAIT_MS = 15000
local MSG_PREPARED    = "공성준비 완료"

local function fail(message)
    log("fatal", "Social lock test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

test_suite {
    name      = "Social Lock Test",
    bot_count = 2,

    on_scenario_finished = function(ctx)
        lib.group.disband(ctx:bot(0))
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)

            if lib.group.form(ctx) == false then
                return fail("could not form the group")
            end
            if run_step(a, "verify_lock_guard", "lock_group", "group", tostring(HOLD_MS)) == false then
                return fail("a group builtin read the group without its guard")
            end

            local packet = a:request(resp.message, protocol.chat(false, "/공성준비"), function(p)
                return p.text ~= nil and p.text:find(MSG_PREPARED, 1, true) ~= nil
            end, MESSAGE_WAIT_MS)
            if packet == nil or packet == false then
                return fail("could not prepare A's clan")
            end
            if run_step(a, "verify_lock_guard", "lock_clan", "clan", tostring(HOLD_MS)) == false then
                return fail("a clan builtin read the clan without its guard")
            end
            return true
        end,
    },
}
