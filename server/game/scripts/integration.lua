-- Integration test scripts for bot-driven scenarios.
-- Add all new integration test Lua logic to this file.

--------------------------------------------------------------------------------
-- Clan tests
--------------------------------------------------------------------------------

local ROLE_DEPUTY = 2

local function cl_chat(me, step, ok, detail)
    if ok then
        me:chat('CL:PASS:' .. step, CHAT_TYPE.NORMAL, false)
    else
        me:chat('CL:FAIL:' .. step .. ':' .. (detail or ''), CHAT_TYPE.NORMAL, false)
    end
end

local function cl_assert(me, step, ok, detail)
    cl_chat(me, step, ok, detail)
    return ok
end

local function get_clan(me)
    local clan = me:clan()
    if clan == nil then
        return nil, 'no clan'
    end
    return clan
end

local function join_member(me, target_name)
    local clan, err = get_clan(me)
    if clan == nil then
        return err
    end
    return clan:join(me, target_name)
end

local function kick_member(me, target_name)
    local clan, err = get_clan(me)
    if clan == nil then
        return err
    end
    return clan:kick(me, target_name)
end

local function change_member_role(me, target_name, role)
    local clan, err = get_clan(me)
    if clan == nil then
        return err
    end
    return clan:change_role(me, target_name, role)
end

local function change_clan_title(me, title)
    local clan, err = get_clan(me)
    if clan == nil then
        return err
    end
    return clan:title(me, title)
end

function run(me, step, ...)
    local args = { ... }

    if step == 'create_clan' then
        if me:clan() ~= nil then
            return cl_assert(me, step, false, 'already in clan')
        end
        local err = me:create_clan(me:name())
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        local clan = me:clan()
        if clan == nil or clan:name() ~= me:name() then
            return cl_assert(me, step, false, 'clan name mismatch')
        end
        return cl_assert(me, step, true)

    elseif step == 'set_title' then
        local title = me:name() .. '타이틀'
        local err = change_clan_title(me, title)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        local clan = me:clan()
        if clan == nil then
            return cl_assert(me, step, false, 'no clan')
        end
        if clan:title() ~= title then
            return cl_assert(me, step, false, 'title mismatch')
        end
        return cl_assert(me, step, true)

    elseif step == 'invite' then
        local target_name = args[1]
        if target_name == nil or target_name == '' then
            return cl_assert(me, step, false, 'missing target')
        end
        local err = join_member(me, target_name)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        return cl_assert(me, step, true)

    elseif step == 'invite_fail' then
        local target_name = args[1]
        if target_name == nil or target_name == '' then
            return cl_assert(me, step, false, 'missing target')
        end
        local err = join_member(me, target_name)
        if err == nil then
            return cl_assert(me, step, false, 'expected invite failure')
        end
        return cl_assert(me, step, true)

    elseif step == 'verify_member' then
        local expected_clan_name = args[1]
        if expected_clan_name == nil or expected_clan_name == '' then
            return cl_assert(me, step, false, 'missing clan name')
        end
        local clan = me:clan()
        if clan == nil then
            return cl_assert(me, step, false, 'not in clan')
        end
        if clan:name() ~= expected_clan_name then
            return cl_assert(me, step, false, 'clan name mismatch')
        end
        return cl_assert(me, step, true)

    elseif step == 'promote_deputy' then
        local target_name = args[1]
        if target_name == nil or target_name == '' then
            return cl_assert(me, step, false, 'missing target')
        end
        local err = change_member_role(me, target_name, ROLE_DEPUTY)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        return cl_assert(me, step, true)

    elseif step == 'change_role_fail' then
        local target_name = args[1]
        local role = tonumber(args[2])
        if target_name == nil or target_name == '' or role == nil then
            return cl_assert(me, step, false, 'missing arguments')
        end
        local err = change_member_role(me, target_name, role)
        if err == nil then
            return cl_assert(me, step, false, 'expected role change failure')
        end
        return cl_assert(me, step, true)

    elseif step == 'change_title' then
        local title = args[1]
        if title == nil or title == '' then
            return cl_assert(me, step, false, 'missing title')
        end
        local err = change_clan_title(me, title)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        local clan = me:clan()
        if clan == nil or clan:title() ~= title then
            return cl_assert(me, step, false, 'title mismatch')
        end
        return cl_assert(me, step, true)

    elseif step == 'change_title_fail' then
        local title = args[1]
        if title == nil or title == '' then
            return cl_assert(me, step, false, 'missing title')
        end
        local err = change_clan_title(me, title)
        if err == nil then
            return cl_assert(me, step, false, 'expected title change failure')
        end
        return cl_assert(me, step, true)

    elseif step == 'kick' then
        local target_name = args[1]
        if target_name == nil or target_name == '' then
            return cl_assert(me, step, false, 'missing target')
        end
        local err = kick_member(me, target_name)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        return cl_assert(me, step, true)

    elseif step == 'kick_fail' then
        local target_name = args[1]
        if target_name == nil or target_name == '' then
            return cl_assert(me, step, false, 'missing target')
        end
        local err = kick_member(me, target_name)
        if err == nil then
            return cl_assert(me, step, false, 'expected kick failure')
        end
        return cl_assert(me, step, true)

    elseif step == 'leave' then
        local clan, err = get_clan(me)
        if clan == nil then
            return cl_assert(me, step, false, err)
        end
        err = clan:leave(me)
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        if me:clan() ~= nil then
            return cl_assert(me, step, false, 'still in clan')
        end
        return cl_assert(me, step, true)

    elseif step == 'destroy_clan' then
        local err = me:destroy_clan()
        if err ~= nil then
            return cl_assert(me, step, false, err)
        end
        if me:clan() ~= nil then
            return cl_assert(me, step, false, 'clan still exists')
        end
        return cl_assert(me, step, true)
    end

    return cl_assert(me, step, false, 'unknown step')
end

--------------------------------------------------------------------------------
-- Marketplace tests
--------------------------------------------------------------------------------

local WEAPON_ITEM = '양첨목봉'
local BUNDLE_ITEM = '도토리'
local CUSTOM_WEAPON_NAME = '마켓테스트'

local WEAPON_LIST_PRICE = 10000
local BUNDLE_UNIT_PRICE = 100
local BUNDLE_LIST_COUNT = 100
local WEAPON_DURABILITY_PERCENT = 50

local LISTING_FEE_RATE = 0.05
local MSG_INSUFFICIENT_LISTING_FEE = '등록 수수료가 부족합니다.'

local function listing_fee(count, price)
    return math.floor(count * price * LISTING_FEE_RATE)
end

local function mp_chat(me, step, ok, detail)
    if ok then
        me:chat('MP:PASS:' .. step, CHAT_TYPE.NORMAL, false)
    else
        me:chat('MP:FAIL:' .. step .. ':' .. (detail or ''), CHAT_TYPE.NORMAL, false)
    end
end

local function mp_assert(me, step, ok, detail)
    mp_chat(me, step, ok, detail)
    return ok
end

local function set_equipment_durability_percent(me, percent)
    for _, item in pairs(me:items()) do
        local model = item:model()
        if model ~= nil and model:attr(ITEM_ATTRIBUTE.EQUIPMENT) then
            local max_durability = model:durability()
            if max_durability ~= nil and max_durability > 0 then
                item:durability(math.floor(max_durability * (percent / 100.0)))
            end
        end
    end
end

local function find_slot_by_name(me, name)
    for slot, item in pairs(me:items()) do
        if item ~= nil then
            local renamed = item:rename()
            if renamed ~= nil and renamed == name then
                return slot
            end
            local model = item:model()
            if model ~= nil and model:name() == name then
                return slot
            end
        end
    end
    return nil
end

local function has_item(me, name)
    return find_slot_by_name(me, name) ~= nil
end

local function item_count(me, name)
    local slot = find_slot_by_name(me, name)
    if slot == nil then
        return 0
    end
    for s, item in pairs(me:items()) do
        if s == slot then
            return item:count()
        end
    end
    return 0
end

local function verify_durability_percent(me, name, expected_percent)
    local slot = find_slot_by_name(me, name)
    if slot == nil then
        return false
    end
    for s, item in pairs(me:items()) do
        if s == slot then
            local model = item:model()
            if model == nil then
                return false
            end
            local max_durability = model:durability()
            local durability = item:durability()
            if max_durability == nil or max_durability <= 0 or durability == nil then
                return false
            end
            local actual = math.floor((durability / max_durability) * 100)
            return actual == expected_percent
        end
    end
    return false
end

local function marketplace_search(me, option)
    return me:marketplace_search(option)
end

local function search_listing_count(me, item_name)
    local err, result = marketplace_search(me, { item_name = item_name, page = 1 })
    if err ~= nil then
        return nil, err
    end
    if result == nil or result.listings == nil then
        return 0
    end
    local count = 0
    for _, listing in ipairs(result.listings) do
        local model = id2item(listing.item_data.model)
        if model ~= nil then
            count = count + 1
        end
    end
    return count
end

local function find_listing_by_model_name(me, item_name, seller_id)
    local option = { page = 1 }
    if item_name ~= nil then
        option.item_name = item_name
    end
    if seller_id ~= nil then
        option.seller_id = seller_id
    end
    local err, result = marketplace_search(me, option)
    if err ~= nil then
        return nil, err
    end
    if result == nil or result.listings == nil then
        return nil
    end
    for _, listing in ipairs(result.listings) do
        local model = id2item(listing.item_data.model)
        if model ~= nil and (item_name == nil or model:name() == item_name) then
            return listing
        end
    end
    return nil
end

local function receive_first_storage_reward(me)
    sleep(2000)
    local entries = me:storage_entries()
    if entries == nil or #entries == 0 then
        return false
    end
    for _, entry in ipairs(entries) do
        if entry.received == false then
            return me:receive_storage_reward(entry.id)
        end
    end
    return false
end

local function list_item(me, item_name, count, price)
    local slot = find_slot_by_name(me, item_name)
    if slot == nil then
        return 'item not found: ' .. item_name
    end
    return me:marketplace_list(slot - 1, count, price)
end

function run_a(me, step)
    if step == 'verify_empty' then
        local weapon_count, weapon_err = search_listing_count(me, WEAPON_ITEM)
        if weapon_err ~= nil then
            return mp_assert(me, step, false, weapon_err)
        end
        local bundle_count, bundle_err = search_listing_count(me, BUNDLE_ITEM)
        if bundle_err ~= nil then
            return mp_assert(me, step, false, bundle_err)
        end
        return mp_assert(me, step, weapon_count == 0 and bundle_count == 0)
    elseif step == 'prepare_weapon' then
        if me:mkitem(WEAPON_ITEM, 1) == nil then
            return mp_assert(me, step, false, 'mkitem failed')
        end
        local item = me:item(WEAPON_ITEM)
        if item == nil then
            return mp_assert(me, step, false, 'weapon not found')
        end
        item:rename(CUSTOM_WEAPON_NAME)
        set_equipment_durability_percent(me, WEAPON_DURABILITY_PERCENT)
        if not has_item(me, CUSTOM_WEAPON_NAME) then
            return mp_assert(me, step, false, 'rename failed')
        end
        if not verify_durability_percent(me, CUSTOM_WEAPON_NAME, WEAPON_DURABILITY_PERCENT) then
            return mp_assert(me, step, false, 'durability mismatch')
        end
        return mp_assert(me, step, true)
    elseif step == 'list_weapon_fee_fail' then
        me:money(0)
        local err = list_item(me, CUSTOM_WEAPON_NAME, 1, WEAPON_LIST_PRICE)
        if err == nil then
            return mp_assert(me, step, false, 'expected listing fee failure')
        end
        if err:find(MSG_INSUFFICIENT_LISTING_FEE) == nil then
            return mp_assert(me, step, false, err)
        end
        return mp_assert(me, step, true)
    elseif step == 'list_weapon_ok' then
        local fee = listing_fee(1, WEAPON_LIST_PRICE)
        me:money(fee + 1000)
        local err = list_item(me, CUSTOM_WEAPON_NAME, 1, WEAPON_LIST_PRICE)
        if err ~= nil then
            return mp_assert(me, step, false, err)
        end
        if has_item(me, CUSTOM_WEAPON_NAME) or has_item(me, WEAPON_ITEM) then
            return mp_assert(me, step, false, 'weapon still in inventory')
        end
        return mp_assert(me, step, true)
    elseif step == 'verify_weapon_count' then
        local count, err = search_listing_count(me, WEAPON_ITEM)
        if err ~= nil then
            return mp_assert(me, step, false, err)
        end
        return mp_assert(me, step, count == 1)
    elseif step == 'verify_two_listings' then
        local weapon_count, weapon_err = search_listing_count(me, WEAPON_ITEM)
        if weapon_err ~= nil then
            return mp_assert(me, step, false, weapon_err)
        end
        local bundle_count, bundle_err = search_listing_count(me, BUNDLE_ITEM)
        if bundle_err ~= nil then
            return mp_assert(me, step, false, bundle_err)
        end
        return mp_assert(me, step, weapon_count == 1 and bundle_count == 1)
    elseif step == 'cancel_relist' then
        local listing = find_listing_by_model_name(me, WEAPON_ITEM, me:uid())
        if listing == nil then
            return mp_assert(me, step, false, 'weapon listing not found')
        end
        local cancel_err = me:marketplace_cancel(listing.id)
        if cancel_err ~= nil then
            return mp_assert(me, step, false, cancel_err)
        end
        if not receive_first_storage_reward(me) then
            return mp_assert(me, step, false, 'storage reward failed after cancel')
        end
        if not has_item(me, CUSTOM_WEAPON_NAME) then
            return mp_assert(me, step, false, 'weapon not restored')
        end
        if not verify_durability_percent(me, CUSTOM_WEAPON_NAME, WEAPON_DURABILITY_PERCENT) then
            return mp_assert(me, step, false, 'durability not preserved')
        end
        local empty_count, empty_err = search_listing_count(me, WEAPON_ITEM)
        if empty_err ~= nil then
            return mp_assert(me, step, false, empty_err)
        end
        if empty_count ~= 0 then
            return mp_assert(me, step, false, 'weapon listing still exists')
        end
        local fee = listing_fee(1, WEAPON_LIST_PRICE)
        me:money(me:money() + fee)
        local list_err = list_item(me, CUSTOM_WEAPON_NAME, 1, WEAPON_LIST_PRICE)
        if list_err ~= nil then
            return mp_assert(me, step, false, list_err)
        end
        local weapon_count, weapon_err = search_listing_count(me, WEAPON_ITEM)
        if weapon_err ~= nil then
            return mp_assert(me, step, false, weapon_err)
        end
        local bundle_count, bundle_err = search_listing_count(me, BUNDLE_ITEM)
        if bundle_err ~= nil then
            return mp_assert(me, step, false, bundle_err)
        end
        return mp_assert(me, step, weapon_count == 1 and bundle_count == 1)
    elseif step == 'purchase_bundle' then
        me:money(BUNDLE_LIST_COUNT * BUNDLE_UNIT_PRICE + 1000)
        local listing = find_listing_by_model_name(me, BUNDLE_ITEM)
        if listing == nil then
            return mp_assert(me, step, false, 'bundle listing not found')
        end
        local purchase_err = me:marketplace_purchase(listing.id, BUNDLE_LIST_COUNT)
        if purchase_err ~= nil then
            return mp_assert(me, step, false, purchase_err)
        end
        if not receive_first_storage_reward(me) then
            return mp_assert(me, step, false, 'storage reward failed after purchase')
        end
        if item_count(me, BUNDLE_ITEM) ~= BUNDLE_LIST_COUNT then
            return mp_assert(me, step, false, 'bundle count mismatch')
        end
        return mp_assert(me, step, true)
    elseif step == 'verify_empty_final' then
        local weapon_count, weapon_err = search_listing_count(me, WEAPON_ITEM)
        if weapon_err ~= nil then
            return mp_assert(me, step, false, weapon_err)
        end
        local bundle_count, bundle_err = search_listing_count(me, BUNDLE_ITEM)
        if bundle_err ~= nil then
            return mp_assert(me, step, false, bundle_err)
        end
        return mp_assert(me, step, weapon_count == 0 and bundle_count == 0)
    end
    return mp_assert(me, step, false, 'unknown step')
end

function run_b(me, step)
    if step == 'prepare_bundle' then
        if me:mkitem(BUNDLE_ITEM, 200) == nil then
            return mp_assert(me, step, false, 'mkitem failed')
        end
        return mp_assert(me, step, item_count(me, BUNDLE_ITEM) == 200)
    elseif step == 'list_bundle' then
        local fee = listing_fee(BUNDLE_LIST_COUNT, BUNDLE_UNIT_PRICE)
        me:money(fee + 1000)
        local err = list_item(me, BUNDLE_ITEM, BUNDLE_LIST_COUNT, BUNDLE_UNIT_PRICE)
        if err ~= nil then
            return mp_assert(me, step, false, err)
        end
        if item_count(me, BUNDLE_ITEM) ~= 100 then
            return mp_assert(me, step, false, 'bundle inventory mismatch')
        end
        return mp_assert(me, step, true)
    elseif step == 'purchase_weapon' then
        me:money(WEAPON_LIST_PRICE + 1000)
        local listing = find_listing_by_model_name(me, WEAPON_ITEM)
        if listing == nil then
            return mp_assert(me, step, false, 'weapon listing not found')
        end
        local purchase_err = me:marketplace_purchase(listing.id, 1)
        if purchase_err ~= nil then
            return mp_assert(me, step, false, purchase_err)
        end
        if not receive_first_storage_reward(me) then
            return mp_assert(me, step, false, 'storage reward failed after purchase')
        end
        if not has_item(me, CUSTOM_WEAPON_NAME) then
            return mp_assert(me, step, false, 'custom weapon name missing')
        end
        if not verify_durability_percent(me, CUSTOM_WEAPON_NAME, WEAPON_DURABILITY_PERCENT) then
            return mp_assert(me, step, false, 'durability not preserved')
        end
        return mp_assert(me, step, true)
    end
    return mp_assert(me, step, false, 'unknown step')
end
