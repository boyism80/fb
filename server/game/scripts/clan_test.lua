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
