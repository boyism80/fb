local lib = require("integration.lib")

local BOT1_TITLE    = "Bot1 Article Title"
local BOT1_CONTENTS = "Bot1's test article contents."
local BOT2_TITLE    = "Bot2 Article Title"
local BOT2_CONTENTS = "Bot2's test article contents."
local MAIL_TITLE    = "Mail Test Title"
local MAIL_CONTENTS = "This is a mail test."

local function progress(bot, message)
    log("debug", message)
    bot:chat("=== " .. message .. " ===")
end

local function find_article_by_name(articles, uname)
    for _, article in ipairs(articles) do
        if article.uname == uname then
            return article
        end
    end
    return nil
end

local function find_mail_by_sender(mails, sender_name)
    for _, mail in ipairs(mails) do
        if mail.sender_name == sender_name then
            return mail
        end
    end
    return nil
end

test_suite {
    name      = "Bulletin Test",
    bot_count = 2,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "BULLETIN TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            progress(bot1, "TESTING GETTING BULLETIN SECTIONS")

            local sections = bot1:bulletin_get_sections()
            if sections == nil or #sections == 0 then
                progress(bot1, "FAILED TO GET BULLETIN SECTIONS")
                return false
            end

            progress(bot1, "SUCCESSFULLY RETRIEVED " .. #sections .. " SECTIONS")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            progress(bot1, "TESTING BULLETIN WRITE WITH LOW LEVEL (SHOULD FAIL)")
            if bot1:bulletin_write(1, BOT1_TITLE, BOT1_CONTENTS) then
                progress(bot1, "BULLETIN WRITE TEST FAILED - Should have failed with low level")
                return false
            end
            progress(bot1, "BULLETIN WRITE FAILED AS EXPECTED (LOW LEVEL)")

            progress(bot1, "CHANGING LEVEL TO 10 FOR BOTH BOTS")
            bot1:level(10)
            bot2:level(10)

            progress(bot1, "LEVEL CHANGED TO 10, TESTING BULLETIN WRITE AGAIN")
            if bot1:bulletin_write(1, BOT1_TITLE, BOT1_CONTENTS) == false then
                progress(bot1, "BULLETIN WRITE TEST FAILED FOR BOT1 WITH LEVEL 10")
                return false
            end
            progress(bot1, "BOT1 WRITE TEST COMPLETED SUCCESSFULLY WITH LEVEL 10")

            if bot2:bulletin_write(1, BOT2_TITLE, BOT2_CONTENTS) == false then
                progress(bot2, "BULLETIN WRITE TEST FAILED FOR BOT2 WITH LEVEL 10")
                return false
            end
            progress(bot2, "BOT2 WRITE TEST COMPLETED SUCCESSFULLY WITH LEVEL 10")

            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            progress(bot1, "TESTING BULLETIN ARTICLE LIST TO FIND OUR ARTICLE")
            local articles = bot1:bulletin_get_articles(1, 0x7FFF)
            local my_article = find_article_by_name(articles, bot1:name())
            if my_article == nil then
                progress(bot1, "FAILED TO FIND OUR WRITTEN ARTICLE")
                return false
            end
            progress(bot1, "FOUND OUR WRITTEN ARTICLE - ID: " .. tostring(my_article.id))

            progress(bot1, "TESTING BULLETIN ARTICLE READ")
            local read = bot1:bulletin_read_article(1, my_article.id)
            if read.title ~= BOT1_TITLE or read.contents ~= BOT1_CONTENTS then
                progress(bot1, "FAILED TO READ OUR OWN ARTICLE")
                return false
            end
            progress(bot1, "BULLETIN ARTICLE READ TEST COMPLETED SUCCESSFULLY")

            progress(bot1, "TESTING WITH ANOTHER USER'S ARTICLE")
            articles = bot1:bulletin_get_articles(1, 0x7FFF)
            local other_article = find_article_by_name(articles, bot2:name())
            if other_article ~= nil then
                progress(bot1, "FOUND OTHER'S ARTICLE - ID: " .. tostring(other_article.id))
                progress(bot1, "TESTING READ OTHER'S ARTICLE")
                local other_read = bot1:bulletin_read_article(1, other_article.id)
                if other_read.title ~= BOT2_TITLE or other_read.contents ~= BOT2_CONTENTS then
                    progress(bot1, "FAILED TO READ OTHER'S ARTICLE")
                    return false
                end
                progress(bot1, "READ OTHER'S ARTICLE SUCCESSFUL")
            else
                progress(bot1, "SKIPPING: NO OTHER USER ARTICLE FOUND")
            end

            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            progress(bot1, "TESTING BULLETIN ARTICLE DELETE")
            local articles = bot1:bulletin_get_articles(1, 0x7FFF)
            local my_article = find_article_by_name(articles, bot1:name())
            if my_article == nil then
                progress(bot1, "FAILED TO FIND OUR WRITTEN ARTICLE FOR DELETE TEST")
                return false
            end
            progress(bot1, "FOUND OUR WRITTEN ARTICLE FOR DELETE TEST - ID: " .. tostring(my_article.id))

            if bot1:bulletin_delete_article(1, my_article.id) == false then
                progress(bot1, "FAILED TO DELETE OUR OWN ARTICLE")
                return false
            end
            progress(bot1, "BULLETIN ARTICLE DELETE TEST COMPLETED SUCCESSFULLY")

            progress(bot1, "TESTING DELETING NON-EXISTENT ARTICLE (SHOULD FAIL)")
            if bot1:bulletin_delete_article(1, my_article.id) then
                progress(bot1, "FAILED: DELETING NON-EXISTENT ARTICLE SUCCEEDED (UNEXPECTED)")
                return false
            end
            progress(bot1, "SUCCESS: FAILED TO DELETE NON-EXISTENT ARTICLE AS EXPECTED")

            articles = bot1:bulletin_get_articles(1, 0x7FFF)
            local other_article = find_article_by_name(articles, bot2:name())
            if other_article ~= nil then
                progress(bot1, "FOUND OTHER'S ARTICLE - ID: " .. tostring(other_article.id))
                progress(bot1, "TESTING DELETE OTHER'S ARTICLE (SHOULD FAIL)")
                if bot1:bulletin_delete_article(1, other_article.id) then
                    progress(bot1, "FAILED: DELETING OTHER'S ARTICLE SUCCEEDED (UNEXPECTED)")
                    return false
                end
                progress(bot1, "SUCCESS: FAILED TO DELETE OTHER'S ARTICLE AS EXPECTED")
            end

            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            progress(bot1, "TESTING MAIL FUNCTIONALITY")
            progress(bot1, "SENDING MAIL FROM " .. bot1:name() .. " TO " .. bot2:name())
            if bot1:bulletin_send_mail(bot2:name(), MAIL_TITLE, MAIL_CONTENTS) == false then
                progress(bot1, "FAILED TO SEND MAIL")
                return false
            end

            ctx:sleep(1000)

            progress(bot2, "CHECKING " .. bot2:name() .. "'S MAILBOX")
            local mails = bot2:bulletin_get_mails()
            local mail = find_mail_by_sender(mails, bot1:name())
            if mail == nil then
                progress(bot2, "MAIL NOT FOUND IN RECEIVER'S MAILBOX")
                return false
            end
            progress(bot2, "MAIL FOUND WITH ID: " .. tostring(mail.id))

            progress(bot2, "READING MAIL " .. tostring(mail.id))
            local read = bot2:bulletin_read_mail(mail.id)
            if read.title ~= MAIL_TITLE or read.contents ~= MAIL_CONTENTS then
                progress(bot2, "FAILED TO READ MAIL WITH CORRECT CONTENT")
                return false
            end

            progress(bot2, "DELETING MAIL " .. tostring(mail.id))
            if bot2:bulletin_delete_mail(mail.id) == false then
                progress(bot2, "FAILED TO DELETE MAIL")
                return false
            end
            progress(bot2, "MAIL DELETED SUCCESSFULLY")

            progress(bot2, "TESTING DELETING NON-EXISTENT MAIL (SHOULD FAIL)")
            if bot2:bulletin_delete_mail(mail.id) then
                progress(bot2, "FAILED: DELETING NON-EXISTENT MAIL SUCCEEDED (UNEXPECTED)")
                return false
            end
            progress(bot2, "SUCCESS: FAILED TO DELETE NON-EXISTENT MAIL AS EXPECTED")

            return true
        end,
    },
}
