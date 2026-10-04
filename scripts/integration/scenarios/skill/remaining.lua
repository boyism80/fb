local spell_runner = require("integration.lib.spell_runner")
local script_step  = require("integration.lib.script_step")
local skill        = require("integration.lib.skill")
local lib          = require("integration.lib")

local M = {}

local resp     = require("integration.response")
local protocol = require("integration.protocol")

local SCRIPT = "test/integration"

local DRAGON_STRIKES = {
    "용마제일격", "용마제이격", "용마제삼격", "용마제사격", "용마제오격",
    "용마제육격", "용마제칠격", "용마제팔격", "용마제구격",
    "용천제일격", "용천제이격", "용천제삼격", "용천제사격", "용천제오격",
    "용천제육격", "용천제칠격", "용천제팔격", "용천제구격",
}

local PALM_STRIKES = {
    { name = "일성백열장", mp = 180 },
    { name = "이성백열장", mp = 300 },
    { name = "삼성백열장", mp = 420 },
    { name = "사성백열장", mp = 550 },
    { name = "오성백열장", mp = 700 },
}

local KICKS = {
    { name = "금강퇴", hp = 120 },
    { name = "일성금강퇴", hp = 240 },
    { name = "이성금강퇴", hp = 480 },
    { name = "삼성금강퇴", hp = 960 },
    { name = "사성금강퇴", hp = 1920 },
    { name = "오성금강퇴", hp = 3840 },
}

local CURSES = {
    { name = "용의제일주", delta = 31 },
    { name = "용의제이주", delta = 35 },
    { name = "용의제삼주", delta = 37 },
    { name = "용의제사주", delta = 45 },
    { name = "용의제오주", delta = 50 },
    { name = "용의제육주", delta = 60 },
    { name = "용의제칠주", delta = 63 },
    { name = "용의제팔주", delta = 65 },
    { name = "용의제구주", delta = 80 },
    { name = "용의제일노", delta = 51 },
    { name = "용의제이노", delta = 55 },
    { name = "용의제삼노", delta = 57 },
    { name = "용의제사노", delta = 63 },
    { name = "용의제오노", delta = 68 },
    { name = "용의제육노", delta = 72 },
    { name = "용의제칠노", delta = 63 },
    { name = "용의제팔노", delta = 65 },
    { name = "용의제구노", delta = 80 },
}

local MOVES = {
    { name = "강제이동(상)", dx = 0, dy = -1 },
    { name = "강제이동(하)", dx = 0, dy = 1 },
    { name = "강제이동(좌)", dx = -1, dy = 0 },
    { name = "강제이동(우)", dx = 1, dy = 0 },
}

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, SCRIPT, func, step, ...)
end

local function fail(fmt, ...)
    log("fatal", string.format(fmt, ...))
    return false
end

-- Spell scripts send the cast message before the buff packet, so the buff state lags the response.
local function wait_buff(bot, name, expected)
    return lib.wait.state(bot, resp.spell_buff, function()
        return bot:has_buff(name) == expected
    end, 1500)
end

local function request_phydef(bot)
    local info = bot:request(
        resp.internal_info,
        protocol.self_info(),
        function(_)
            return true
        end)
    if info == false or info == nil or info.phydef == nil then
        return nil
    end
    return info.phydef
end

local function full_hp(bot)
    bot:set_max_hp_mp(5000000, 5000000)
    bot:set_current_hp_mp(5000000, 5000000)
end

local function expect_lua_error(ctx, bot)
    local saw = false
    ctx:hook("message", function(_, _, packet)
        local text = packet.text or ""
        if text:find("LUAERR", 1, true) ~= nil and text:find("integration boom", 1, true) ~= nil then
            saw = true
        end
    end)

    bot:chat("/스크립트 test/integration explode boom")
    for _ = 1, 30 do
        if saw then
            break
        end
        ctx:sleep(100)
    end
    ctx:unhook("message")

    if saw == false then
        return fail("admin bot did not receive LUAERR for a script error")
    end
    return true
end

local function move_cases()
    local cases = {}
    for _, move in ipairs(MOVES) do
        table.insert(cases, {
            name = move.name,
            response = resp.position,
            cast_type = "NORMAL",
            pre = function(caster, _, state)
                local pos = caster:position()
                state.x = pos[1] + move.dx
                state.y = pos[2] + move.dy
            end,
            condition = function(packet, _, _, state)
                if packet.abs == nil then
                    return nil
                end
                if packet.abs[1] ~= state.x or packet.abs[2] ~= state.y then
                    return nil
                end
                return true
            end,
        })
    end
    return cases
end

local function map_id_case()
    return {
        name = "맵번호",
        response = resp.message,
        cast_type = "INPUT",
        message = function(caster)
            local model = id2map(caster:map())
            if model == nil then
                return ""
            end
            return model:name()
        end,
        pre = function(caster, _, state)
            local model = id2map(caster:map())
            if model == nil then
                return false
            end
            state.map_id = tostring(model:id())
        end,
        condition = function(packet, _, _, state)
            if packet.type ~= "POPUP" or packet.text == nil then
                return nil
            end
            if packet.text:find(state.map_id, 1, true) == nil then
                return nil
            end
            return true
        end,
    }
end

local function hide_case()
    return {
        name = "잠복근무",
        cast = function(caster, _, slot)
            caster:send(protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}))
            caster:sleep(200)
            return true
        end,
        post = function(caster)
            if run_step(caster, "verify_super_hide", "hide_on", "1") ~= true then
                return false
            end
            return run_step(caster, "clear_super_hide", "hide_off") == true
        end,
    }
end

local function info_popup_case(name, cast_type, place_front)
    return {
        name = name,
        response = resp.message,
        cast_type = cast_type,
        pre = function(caster, target, state)
            full_hp(caster)
            state.expected_mp = caster:mp() - 30
            state.target_name = target:name()
            if place_front then
                local pos = caster:position()
                local model = id2map(caster:map())
                if model == nil then
                    return false
                end
                target:map_move(model:name(), pos[1], pos[2] + 1, state.slot)
                caster:direction("BOTTOM")
            end
        end,
        condition = function(packet, _, _, state)
            if packet.type ~= "POPUP" or packet.text == nil then
                return nil
            end
            if packet.text:find(state.target_name, 1, true) == nil then
                return nil
            end
            if packet.text:find("레벨", 1, true) == nil then
                return nil
            end
            if packet.text:find("힘", 1, true) == nil then
                return nil
            end
            return true
        end,
        post = function(caster, target, state)
            if place_front then
                local model = id2map(caster:map())
                if model ~= nil then
                    target:map_move(model:name(), state.home_x, state.home_y, state.slot)
                end
            end
            return caster:mp() == state.expected_mp
        end,
    }
end

local function heal_self_case()
    return {
        name = "운공체식",
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:set_max_hp_mp(5000000, 5000000)
            caster:set_current_hp_mp(1000, 10000)
            state.expected_hp = 11000
            state.expected_mp = 0
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp or packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
    }
end

local function hp_drop_case(name, mp)
    return {
        name = name,
        cast = function(caster, target, slot, state)
            local result = caster:request_on(
                target,
                resp.update_internal,
                protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
                function(packet)
                    return packet.ch_hp ~= nil and packet.ch_hp < state.before_hp
                end)
            if result == false or result == nil then
                return false
            end
            if mp ~= nil and caster:mp() ~= state.expected_mp then
                return false
            end
            return true
        end,
        pre = function(caster, target, state)
            full_hp(caster)
            full_hp(target)
            state.before_hp = target:hp()
            if mp ~= nil then
                state.expected_mp = caster:mp() - mp
            end
        end,
    }
end

local function kick_case(name, hp_cost)
    return {
        name = name,
        response = resp.update_internal,
        cast_type = "NORMAL",
        pre = function(caster, _, state)
            caster:chat("/몬스터범위제거 3")
            caster:direction("BOTTOM")
            full_hp(caster)
            state.mob = caster:spawn_monster_relative("다람쥐", 0, 1)
            if state.mob == nil or state.mob.oid == 0 then
                return false
            end
            state.expected_hp = caster:hp() - hp_cost
        end,
        condition = function(packet, _, _, state)
            if packet.ch_hp ~= state.expected_hp then
                return nil
            end
            return true
        end,
    }
end

local function curse_case(name, delta, mp)
    return {
        name = name,
        response = resp.update_internal,
        cast_type = "TARGET",
        reset_buffs = false,
        pre = function(caster, target, state)
            full_hp(caster)
            target:remove_buffs()
            state.expected_mp = caster:mp() - mp
            state.base = request_phydef(target)
            if state.base == nil then
                return false
            end
        end,
        condition = function(packet, _, _, state)
            if packet.ch_mp ~= state.expected_mp then
                return nil
            end
            return true
        end,
        post = function(_, target, state)
            local buffed = request_phydef(target)
            if buffed ~= state.base + delta then
                return false
            end
            target:remove_buffs()
            return request_phydef(target) == state.base
        end,
    }
end

local function build_named(list, builder)
    local cases = {}
    for _, entry in ipairs(list) do
        table.insert(cases, builder(entry))
    end
    return cases
end

local function cast_neutralize(caster, target)
    caster:chat("/지력바꾸기 255")
    caster:remove_buffs()
    target:remove_buffs()
    caster:sleep(200)

    local arm = caster:learn_spell("시약무장")
    if arm == 0xFF then
        return fail("무력화: failed to learn 시약무장")
    end

    full_hp(caster)
    local armed = caster:request(
        resp.message,
        protocol.spell_cast("TARGET", arm, "", target:oid(), target:position()),
        function(packet)
            return skill.is_cast_ready(packet.text, "시약무장")
        end)
    if armed == false or armed == nil or wait_buff(target, "시약무장", true) == false then
        return fail("무력화: 시약무장 did not land")
    end

    local slot = caster:learn_spell("무력화")
    if slot == 0xFF then
        return fail("무력화: failed to learn")
    end

    for _ = 1, 8 do
        full_hp(caster)
        if target:has_buff("시약무장") ~= true then
            caster:request(
                resp.message,
                protocol.spell_cast("TARGET", arm, "", target:oid(), target:position()),
                function(packet)
                    return skill.is_cast_ready(packet.text, "시약무장")
                end)
        end

        local before = caster:mp()
        local packet = caster:request(
            resp.message,
            protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
            function(p)
                local text = p.text or ""
                if skill.is_cast_ready(text, "무력화") then
                    return true
                end
                if text:find("무력화 실패", 1, true) ~= nil then
                    return true
                end
                return false
            end)
        if packet ~= false and packet ~= nil and skill.is_cast_ready(packet.text, "무력화") then
            if run_step(caster, "verify_no_buff", "neutralize_server", target:name(), "시약무장") ~= true then
                return fail("무력화: target buff was not removed on the server")
            end
            if wait_buff(target, "시약무장", false) == false then
                return fail("무력화: server removed the buff but the target client still shows it")
            end
            if caster:mp() ~= before - 500 then
                return fail("무력화: mp cost mismatch")
            end
            return true
        end
    end

    return fail("무력화: did not succeed")
end

local function read_trailing_number(text)
    if text == nil then
        return nil
    end
    return tonumber(text:match("(%-?%d+)%s*$"))
end

local function azure_case()
    return {
        name = "청룡마령참",
        cast = function(caster, target, slot)
            local result = caster:request_on(
                target,
                resp.message,
                protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
                function(packet)
                    local text = packet.text or ""
                    if text:find("청룡마령참", 1, true) == nil then
                        return false
                    end
                    if text:find(caster:name(), 1, true) == nil then
                        return false
                    end
                    return true
                end)
            return result ~= false and result ~= nil
        end,
    }
end

local function cast_reagent_arm(caster, target)
    target:remove_buffs()
    local pass, text = run_step(target, "snapshot_buff_phydef", "arm_before")
    local before = pass and read_trailing_number(text) or nil
    if before == nil then
        return fail("시약무장: failed to read buff_phydef")
    end

    local slot = caster:learn_spell("시약무장")
    if slot == 0xFF then
        return fail("시약무장: failed to learn")
    end
    full_hp(caster)
    local expected_mp = caster:mp() - 30
    local casted = caster:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
        function(packet)
            return packet.ch_mp == expected_mp
        end)
    if casted == false or casted == nil then
        return fail("시약무장: cast failed")
    end
    if run_step(target, "verify_buff_phydef", "arm_up", tostring(before - 10)) ~= true then
        target:remove_buffs()
        return fail("시약무장: buff_phydef did not drop")
    end
    target:remove_buffs()
    if run_step(target, "verify_buff_phydef", "arm_back", tostring(before)) ~= true then
        return fail("시약무장: buff_phydef was not restored")
    end
    return true
end

local function cast_freeze(caster, target)
    local rejected = spell_runner.run_cases({
        {
            name = "정지",
            response = resp.message,
            cast_type = "TARGET",
            pre = function(actor, _, state)
                full_hp(actor)
                state.mp = actor:mp()
            end,
            condition = function(packet)
                local text = packet.text or ""
                if text:find("걸리지 않습니다", 1, true) == nil then
                    return nil
                end
                return true
            end,
            post = function(actor, _, state)
                return actor:mp() == state.mp
            end,
        },
    }, caster, target)
    if rejected == false then
        return fail("정지: equal-role cast was not rejected")
    end

    if run_step(caster, "set_role", "role_user", target:name(), "0") ~= true then
        return fail("정지: failed to lower target role")
    end

    local slot = caster:spell_slot("정지")
    full_hp(caster)
    local froze = false
    if slot ~= 0xFF then
        local ok, packet = pcall(function()
            return caster:request_on(
                target,
                resp.freeze,
                protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
                function(p)
                    return p.enable == true
                end)
        end)
        froze = ok and packet ~= false and packet ~= nil
    end

    local restored = run_step(caster, "set_role", "role_admin", target:name(), "2") == true
    target:remove_buffs()
    target:sleep(200)

    if froze == false then
        return fail("정지: lower-role target was not frozen")
    end
    if restored == false then
        return fail("정지: failed to restore target role")
    end
    return true
end

local function cast_reagent_guard(caster, target)
    local before_text_ok, text = run_step(target, "snapshot_derate", "derate_before")
    local before = before_text_ok and text and tonumber(text:match("(%d+)%s*$")) or nil
    if before == nil then
        return fail("시약보호: failed to read damage_derate")
    end

    local slot = caster:learn_spell("시약보호")
    if slot == 0xFF then
        return fail("시약보호: failed to learn")
    end
    full_hp(caster)
    local expected_mp = caster:mp() - 30
    local casted = caster:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", slot, "", target:oid(), target:position()),
        function(packet)
            return packet.ch_mp == expected_mp
        end)
    if casted == false or casted == nil then
        return fail("시약보호: cast failed")
    end
    if run_step(target, "verify_derate", "derate_up", tostring(before + 1000)) ~= true then
        target:remove_buffs()
        return fail("시약보호: damage_derate did not increase")
    end
    target:remove_buffs()
    if run_step(target, "verify_derate", "derate_back", tostring(before)) ~= true then
        return fail("시약보호: damage_derate was not restored")
    end
    return true
end

function M.run(ctx, caster_index, target_index)
    local caster = ctx:bot(caster_index)
    local target = ctx:bot(target_index)
    local home = target:position()
    local caster_home = caster:position()
    local map_model = id2map(caster:map())

    caster:clear_all_spells()
    target:clear_all_spells()
    caster:remove_buffs()
    target:remove_buffs()
    full_hp(caster)
    full_hp(target)
    caster:direction("BOTTOM")

    if lib.option.disable_pk_protect(caster) == false or lib.option.disable_pk_protect(target) == false then
        return fail("remaining: failed to disable PK_PROTECT")
    end

    if expect_lua_error(ctx, caster) == false then
        return false
    end

    caster:clear_all_spells()
    if spell_runner.run_cases(move_cases(), caster, nil) == false then
        return false
    end
    caster:clear_all_spells()
    if spell_runner.run_cases({ map_id_case(), hide_case() }, caster, nil) == false then
        return false
    end

    local nuri = info_popup_case("누리의빛", "NORMAL", true)
    local bada = info_popup_case("바다의빛", "TARGET", false)
    local function stamp(case)
        case.pre = (function(original)
            return function(actor, other, state)
                state.slot = ctx:suite_slot()
                state.home_x = home[1]
                state.home_y = home[2]
                return original(actor, other, state)
            end
        end)(case.pre)
        return case
    end

    caster:clear_all_spells()
    if spell_runner.run_cases({
        stamp(nuri),
        stamp(bada),
        heal_self_case(),
        azure_case(),
        hp_drop_case("월아일격", nil),
    }, caster, target) == false then
        return false
    end

    caster:clear_all_spells()
    if spell_runner.run_cases(build_named(DRAGON_STRIKES, function(name)
        return hp_drop_case(name, 100)
    end), caster, target) == false then
        return false
    end

    caster:clear_all_spells()
    if spell_runner.run_cases(build_named(PALM_STRIKES, function(entry)
        return hp_drop_case(entry.name, entry.mp)
    end), caster, target) == false then
        return false
    end

    caster:clear_all_spells()
    if spell_runner.run_cases(build_named(KICKS, function(entry)
        return kick_case(entry.name, entry.hp)
    end), caster, nil) == false then
        return false
    end
    caster:chat("/몬스터범위제거 5")

    caster:clear_all_spells()
    local curse_cases = build_named(CURSES, function(entry)
        return curse_case(entry.name, entry.delta, 40)
    end)
    if spell_runner.run_cases(curse_cases, caster, target) == false then
        return false
    end

    if cast_reagent_arm(caster, target) == false then
        return false
    end
    if cast_reagent_guard(caster, target) == false then
        return false
    end
    if cast_neutralize(caster, target) == false then
        return false
    end
    caster:clear_all_spells()
    if cast_freeze(caster, target) == false then
        return false
    end

    caster:clear_all_spells()
    if spell_runner.run_cases({
        {
            name = "의태",
            response = resp.message,
            cast_type = "TARGET",
            condition = function(packet)
                if skill.is_cast_ready(packet.text, "의태") then
                    return true
                end
                return nil
            end,
            post = function(actor)
                if run_step(actor, "verify_mimic", "mimic") ~= true then
                    return false
                end
                actor:remove_buffs()
                return true
            end,
        },
        {
            name = "귀염추혼소",
            pre = function(_, other, state)
                other:remove_buffs()
                local pass, text = run_step(other, "snapshot_buff_phydef", "soul_before")
                state.base = pass and read_trailing_number(text) or nil
                if state.base == nil then
                    return false
                end
            end,
            cast = function(caster, other, slot, state)
                caster:send(protocol.spell_cast("NORMAL", slot, "", 0, {0, 0}))
                if wait_buff(other, "귀염추혼소", true) == false then
                    return false
                end
                return run_step(other, "verify_buff_phydef", "soul_up", tostring(state.base + 50)) == true
            end,
            post = function(_, other, state)
                other:remove_buffs()
                return run_step(other, "verify_buff_phydef", "soul_back", tostring(state.base)) == true
            end,
        },
        {
            name = "다리밟기",
            response = resp.message,
            cast_type = "TARGET",
            condition = function(packet)
                if skill.is_cast_ready(packet.text, "다리밟기") then
                    return true
                end
                return nil
            end,
            post = function(_, other)
                if wait_buff(other, "다리밟기", true) == false then
                    return false
                end
                return run_step(other, "verify_concast_heal", "bridge_heal", "다리밟기") == true
            end,
        },
    }, caster, target) == false then
        return false
    end

    if run_step(caster, "verify_concast_heal", "twinkle", "반짝반짝") ~= true then
        return fail("반짝반짝: concast did not heal")
    end
    if run_step(caster, "verify_buff_roundtrip", "spirit", "신령지익진", "derate") ~= true then
        return fail("신령지익진: buff stat was not restored")
    end
    if run_step(caster, "verify_buff_roundtrip", "power", "파력무참진", "rates") ~= true then
        return fail("파력무참진: buff stat was not restored")
    end
    if run_step(caster, "verify_buff_roundtrip", "sword", "이기어검술", "phydef") ~= true then
        return fail("이기어검술: buff stat was not restored")
    end
    if run_step(caster, "verify_buff_roundtrip", "formless", "무형술", "phydef") ~= true then
        return fail("무형술: buff stat was not restored")
    end

    -- 견우직녀의집 is hosted by another game server. Locally that server is not running, so the
    -- warp is refused; remotely the caster moves there first so the warp stays on one server.
    local reunion_ok = false
    if localhost() then
        reunion_ok = run_step(caster, "verify_reunion", "reunion", "blocked") == true
    elseif map_model ~= nil then
        caster:transfer(protocol.chat(false, "/맵이동 견우직녀의집 3 3"))
        caster = ctx:bot(caster_index)
        reunion_ok = run_step(caster, "verify_reunion", "reunion", "warp") == true
        caster:transfer(protocol.chat(false, string.format("/맵이동 %s %d %d",
            map_model:name(), caster_home[1], caster_home[2])))
        caster = ctx:bot(caster_index)
    end
    if map_model ~= nil then
        caster:map_move(map_model:name(), caster_home[1], caster_home[2], ctx:suite_slot())
        target:map_move(map_model:name(), home[1], home[2], ctx:suite_slot())
    end
    if reunion_ok == false then
        return fail("견우직녀축복: unbuff did not change hair and warp")
    end

    caster:remove_buffs()
    target:remove_buffs()
    caster:clear_all_spells()
    target:clear_all_spells()
    return true
end

return M
