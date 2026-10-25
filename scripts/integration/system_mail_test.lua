local MAIL_WAIT_MS   = 5000
local SETTLE_WAIT_MS = 3000
local CONTENTS       = "system_mail_test"

local function fail(message)
    log("fatal", "System mail test: " .. message)
    return false
end

local function find_titles(bot, title)
    local found = {}
    for _, mail in ipairs(bot:bulletin_get_mails() or {}) do
        if mail.title == title then
            table.insert(found, mail)
        end
    end
    return found
end

local function wait_title(ctx, bot, title)
    local waited = 0
    while waited < MAIL_WAIT_MS do
        if #find_titles(bot, title) > 0 then
            return true
        end
        ctx:sleep(500)
        waited = waited + 500
    end
    return false
end

test_suite {
    name      = "System Mail Test",
    bot_count = 2,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = string.format("SM_%d_%d", os.time(), math.random(1000, 9999))
            local expire_date = os.date("%Y-%m-%d %H:%M:%S", os.time() + 600)

            a:chat(string.format("/시스템메일 %s %s %s", title, CONTENTS, expire_date))
            if wait_title(ctx, b, title) == false then
                return fail("the system mail did not arrive")
            end

            ctx:sleep(SETTLE_WAIT_MS)
            local mails = find_titles(b, title)
            if #mails ~= 1 then
                return fail(string.format("the system mail arrived %d times", #mails))
            end

            if b:bulletin_delete_mail(mails[1].id) == false then
                return fail("could not delete the system mail")
            end

            if b:logout() == false then
                return fail("logout")
            end
            if b:login() == false then
                return fail("relogin")
            end
            b = ctx:bot(1)
            ctx:sleep(SETTLE_WAIT_MS)

            if #find_titles(b, title) ~= 0 then
                return fail("the deleted system mail was sent again after relogin")
            end
            return true
        end,
    },
}
