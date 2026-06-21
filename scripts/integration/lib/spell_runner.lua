local protocol = require("integration.protocol")
local skill = require("integration.lib.skill")

local M = {}

M.DEFAULT_MAX_ATTEMPTS = 100

function M.case_names(cases)
    local names = {}
    for _, case in ipairs(cases) do
        table.insert(names, case.name)
    end
    return names
end

local GROUP_ONLY_KEYS = {
    spell = true,
    variants = true,
    slot = true,
    skip = true,
}

local function normalize_variant(variant)
    if type(variant) == "string" then
        return { name = variant, message = variant }
    end
    return variant
end

function M.build_case_from_variant(group, variant)
    variant = normalize_variant(variant)
    local case = {}

    for key, value in pairs(group) do
        if GROUP_ONLY_KEYS[key] ~= true then
            case[key] = value
        end
    end

    for key, value in pairs(variant) do
        if key ~= "name" and value ~= nil then
            case[key] = value
        end
    end

    case.name = string.format("%s/%s", group.spell, variant.name)
    return case
end

function M.spell_group_names(groups)
    local names = {}
    for _, group in ipairs(groups) do
        table.insert(names, group.spell)
    end
    return names
end

function M.run_spell_group(group, caster, target, slot, opts)
    for _, variant in ipairs(group.variants) do
        local case = M.build_case_from_variant(group, variant)
        if not case.skip then
            log("debug", string.format(
                "[spell_runner] case=%s START slot=%d",
                case.name, slot))

            if M.run_case(case, caster, target, slot, opts) == false then
                log("debug", string.format("[spell_runner] case=%s FAILED", case.name))
                return false
            end
        end
    end

    return true
end

function M.run_spell_groups(groups, caster, target, opts)
    opts = opts or {}

    if not opts.skip_learn_spells then
        local names = M.spell_group_names(groups)
        if #names > 0 then
            caster:learn_spells(names)
        end
    end

    local next_slot = opts.start_slot or 0

    for _, group in ipairs(groups) do
        if group.skip then
            next_slot = next_slot + 1
        else
            local slot = group.slot
            if slot == nil then
                slot = next_slot
            end

            log("debug", string.format(
                "[spell_runner] spell=%s START slot=%d",
                group.spell, slot))

            if M.run_spell_group(group, caster, target, slot, opts) == false then
                log("debug", string.format(
                    "[spell_runner] spell=%s FAILED",
                    group.spell))
                return false
            end

            if group.slot == nil then
                next_slot = next_slot + 1
            end
        end
    end

    return true
end

function M.wrap_condition(user_condition)
    local retry = false

    local wrapped = function(packet)
        local result = user_condition(packet)
        if result == true then
            return true
        elseif result == false then
            retry = true
            return true
        else
            return false
        end
    end

    return wrapped, function()
        return retry
    end
end

local function resolve_cast_params(case, caster, target, state)
    local cast_type = case.cast_type or "NORMAL"

    local message = case.message or ""
    if type(message) == "function" then
        message = message(caster, target, state)
    end

    local oid, position
    if case.oid ~= nil then
        oid = case.oid(caster, target, state)
    elseif target ~= nil then
        oid = target:oid()
    else
        oid = 0
    end

    if case.position ~= nil then
        position = case.position(caster, target, state)
    elseif target ~= nil then
        position = target:position()
    else
        position = {0, 0}
    end

    return cast_type, message, oid, position
end

function M.run_case(case, caster, target, slot, opts)
    opts = opts or {}
    local state = {}
    local attempt = 0
    local max_attempts = case.max_attempts or opts.max_attempts or M.DEFAULT_MAX_ATTEMPTS
    local packet = nil
    local cast_type, message, oid, position

    if case.should_skip ~= nil and case.should_skip(caster, target, state) == true then
        log("debug", string.format("[spell_runner] %s skipped by should_skip", case.name))
        return true
    end

    while true do
        attempt = attempt + 1
        if attempt > max_attempts then
            log("debug", string.format(
                "[spell_runner] %s exceeded max attempts (%d)",
                case.name, max_attempts))
            return false
        end

        if case.pre ~= nil then
            local pre_ok = case.pre(caster, target, state)
            if pre_ok == false then
                log("debug", string.format("[spell_runner] %s pre failed", case.name))
                return false
            end
        end

        cast_type, message, oid, position = resolve_cast_params(case, caster, target, state)

        local user_condition = case.condition
        if user_condition == nil then
            log("debug", string.format("[spell_runner] %s missing condition", case.name))
            return false
        end

        local wrapped, should_retry = M.wrap_condition(function(pkt)
            return user_condition(pkt, caster, target, state)
        end)

        log("debug", string.format(
            "[spell_runner] %s attempt=%d slot=%d type=%s oid=%s",
            case.name, attempt, slot, cast_type, tostring(oid)))

        packet = caster:request(
            case.response,
            protocol.spell_cast(cast_type, slot, message, oid, position),
            wrapped)

        if should_retry() then
            log("debug", string.format(
                "[spell_runner] %s retry attempt=%d text=%s",
                case.name, attempt, tostring(packet and packet.text)))
            if target ~= nil and target ~= caster and case.clear_target_buffs_on_retry ~= false then
                target:remove_buffs()
            end
        else
            break
        end
    end

    if case.post ~= nil then
        local post_ok = case.post(caster, target, state, packet, opts.ctx)
        if post_ok == false then
            log("debug", string.format("[spell_runner] %s post failed", case.name))
            return false
        end
    end

    log("debug", string.format("[spell_runner] %s ok attempts=%d", case.name, attempt))
    return true
end

function M.run_cases(cases, caster, target, opts)
    opts = opts or {}

    if not opts.skip_learn_spells then
        local names = M.case_names(cases)
        if #names > 0 then
            caster:learn_spells(names)
        end
    end

    local next_slot = opts.start_slot or 0

    for _, case in ipairs(cases) do
        if case.skip then
            next_slot = next_slot + 1
        else
            local slot = case.slot
            if slot == nil then
                slot = next_slot
            end

            log("debug", string.format("[spell_runner] case=%s START slot=%d", case.name, slot))
            if M.run_case(case, caster, target, slot, opts) == false then
                log("debug", string.format("[spell_runner] case=%s FAILED", case.name))
                return false
            end

            if case.slot == nil then
                next_slot = next_slot + 1
            end
        end
    end

    return true
end

return M
