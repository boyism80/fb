local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local ROLE_USER     = 0
local ROLE_ADMIN    = 2
local COLOR_SELF    = 0x88
local COLOR_OTHER   = 0x0F
local POLL_MS       = 2000
local POLL_COUNT    = 7
local CACHE_WAIT_MS = 11000

local function fail(message)
    log("fatal", "User list cache test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function colors(bot)
    local packet = bot:request(resp.user_list, protocol.user_list(), function()
        return true
    end)
    if packet == nil or packet == false then
        return nil
    end

    local result = {}
    for _, user in ipairs(packet.users) do
        result[user.name] = user.color
    end
    return result
end

test_suite {
    name      = "User List Cache Test",
    bot_count = 2,

    on_scenario_finished = function(ctx)
        local a = ctx:bot(0)
        local b = ctx:bot(1)
        run_step(b, "clear_super_hide", "user_list_cache_unhide")
        run_step(b, "set_role", "user_list_cache_restore", a:name(), tostring(ROLE_ADMIN))
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            local listed = nil
            for _ = 1, POLL_COUNT do
                listed = colors(a)
                if listed ~= nil and listed[a:name()] ~= nil and listed[b:name()] ~= nil then
                    break
                end
                ctx:sleep(POLL_MS)
            end
            if listed == nil or listed[a:name()] == nil or listed[b:name()] == nil then
                return fail("both bots did not appear in the list within the cache window")
            end
            if listed[a:name()] ~= COLOR_SELF or listed[b:name()] ~= COLOR_OTHER then
                return fail(string.format("wrong colors self=%s other=%s",
                    tostring(listed[a:name()]), tostring(listed[b:name()])))
            end

            if run_step(b, "set_role", "user_list_cache_demote", a:name(), tostring(ROLE_USER)) == false then
                return fail("could not demote the viewer")
            end

            local slot = b:learn_spell("잠복근무")
            if slot == 0xFF then
                return fail("failed to learn 잠복근무")
            end
            b:send(protocol.spell_cast("NORMAL", slot, "", 0, { 0, 0 }))
            ctx:sleep(200)
            if run_step(b, "verify_super_hide", "user_list_cache_hide", "1") == false then
                return fail("잠복근무 did not hide")
            end

            ctx:sleep(CACHE_WAIT_MS)

            local seen_by_user = colors(a)
            if seen_by_user == nil then
                return fail("no user list for the demoted viewer")
            end
            if seen_by_user[b:name()] ~= nil then
                return fail("a hidden admin is listed to a user")
            end
            if seen_by_user[a:name()] ~= COLOR_SELF then
                return fail("the viewer is missing from its own list")
            end

            local seen_by_self = colors(b)
            if seen_by_self == nil or seen_by_self[b:name()] ~= COLOR_SELF then
                return fail("the hidden admin does not see itself")
            end
            return true
        end,
    },
}
