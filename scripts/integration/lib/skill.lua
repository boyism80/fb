local M = {}

M.DEFAULT_INTERVAL = 100

function M.spell_names(spells)
    local names = {}
    for _, spell in ipairs(spells) do
        table.insert(names, spell.name)
    end
    return names
end

function M.is_cast_ready(text, spell_name)
    if text == nil then
        return false
    end
    return text:find(spell_name, 1, true) ~= nil
        and text:find("외웠습니다", 1, true) ~= nil
end

function M.verify_hp_mp(bot, expected_hp, expected_mp)
    return bot:hp() == expected_hp and bot:mp() == expected_mp
end

function M.request_update_internal(caster, resp, protocol, spell_type, slot, message, oid, position, expected_hp, expected_mp, label)
    label = label or string.format("slot=%d type=%s", slot, spell_type)

    log("debug", string.format(
        "[skill] %s request begin oid=%s pos=(%s,%s) expected_hp=%s expected_mp=%s",
        label,
        tostring(oid),
        position and tostring(position[1]) or "nil",
        position and tostring(position[2]) or "nil",
        tostring(expected_hp),
        tostring(expected_mp)))

    local result = caster:request(
        resp.update_internal,
        protocol.spell_cast(spell_type, slot, message, oid, position),
        function(packet)
            if expected_hp ~= nil and packet.ch_hp ~= expected_hp then
                log("debug", string.format(
                    "[skill] %s hp mismatch: expected=%s actual=%s",
                    label, tostring(expected_hp), tostring(packet.ch_hp)))
                return false
            end
            if expected_mp ~= nil and packet.ch_mp ~= expected_mp then
                log("debug", string.format(
                    "[skill] %s mp mismatch: expected=%s actual=%s",
                    label, tostring(expected_mp), tostring(packet.ch_mp)))
                return false
            end
            return true
        end)

    log("debug", string.format(
        "[skill] %s request ok caster_hp=%d caster_mp=%d packet_hp=%s packet_mp=%s",
        label,
        caster:hp(),
        caster:mp(),
        tostring(result.ch_hp),
        tostring(result.ch_mp)))

    return result
end

function M.cast_until_ready(caster, resp, protocol, spell_type, slot, target_oid, target_pos, spell_name, opts)
    opts = opts or {}
    local target = opts.target
    local mp_cost = opts.mp_cost or 0

    log("debug", string.format(
        "[skill] %s cast_until_ready begin slot=%d type=%s oid=%s mp_cost=%d",
        spell_name, slot, spell_type, tostring(target_oid), mp_cost))

    local attempt = 0
    while true do
        attempt = attempt + 1
        caster:set_current_hp_mp(10000, 10000)
        if target ~= nil and target ~= caster then
            target:set_current_hp_mp(10000, 10000)
        end

        local before_mp = caster:mp()
        local expected_mp = before_mp - mp_cost

        log("debug", string.format(
            "[skill] %s cast_until_ready attempt=%d slot=%d",
            spell_name, attempt, slot))

        local packet = caster:request(
            resp.message,
            protocol.spell_cast(spell_type, slot, "", target_oid, target_pos),
            function(p)
                return p.text ~= nil
            end)

        if M.is_cast_ready(packet.text, spell_name) then
            if mp_cost > 0 and caster:mp() ~= expected_mp then
                log("debug", string.format(
                    "[skill] %s mp cost mismatch after ready: expected=%d actual=%d",
                    spell_name, expected_mp, caster:mp()))
                error(string.format("MP cost mismatch for '%s'", spell_name))
            end
            log("debug", string.format("[skill] %s cast_until_ready ok attempts=%d", spell_name, attempt))
            return true
        end

        log("debug", string.format("[skill] %s cast_until_ready not ready: %s", spell_name, tostring(packet.text)))
        if target ~= nil then
            target:remove_buffs()
        end
    end
end

function M.positions_equal(a, b)
    return a[1] == b[1] and a[2] == b[2]
end

return M
