-- mob: 토끼
-- Rabbit kill count for 낙랑의두루마리5 (q 228); param "squirrel,rabbit", max 5 each.
-- Also increments quest.QUEST_BEGINNER_PATH (사도) hunt param "squirrel,rabbit" at step 5.
local quest = require('lib.quest')

local GOAL = 5

-- @brief   Increment NAKRANG_HUNT progress when killed mob name matches q param.
local function run_nakrang_hunt(me, mobs)
    local q = me:quest(quest.QUEST_NAKRANG_HUNT)
    if q == nil then
        return
    end
    if mobs[1]:model():name() ~= q:param() then
        return
    end
    q:inc_progress(#mobs)
end

-- @brief   Increment rabbit count in quest.QUEST_NAKRANG5 param "squirrel,rabbit" (step 1 only).
local function run_nakrang5_rabbit(me, count)
    local q5 = me:quest(quest.QUEST_NAKRANG5)
    if q5 == nil or q5:completed() or q5:step() ~= 1 then
        return
    end
    local param = q5:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min(tonumber(a) or 0, GOAL)
    local rabbit = math.min((tonumber(b) or 0) + count, GOAL)
    q5:param(string.format('%d,%d', squirrel, rabbit))
end

-- @brief   Increment rabbit count in quest.QUEST_BEGINNER_PATH param "squirrel,rabbit" (step 5 only, 사도).
local function run_beginner_path_rabbit(me, count)
    local q = me:quest(quest.QUEST_BEGINNER_PATH)
    if q == nil or q:completed() or q:step() ~= 5 then
        return
    end
    local param = q:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min(tonumber(a) or 0, GOAL)
    local rabbit = math.min((tonumber(b) or 0) + count, GOAL)
    q:param(string.format('%d,%d', squirrel, rabbit))
end

return {
    -- on_mob_attack = function(me, you)
    -- end,

    -- on_mob_die = function(me)
    -- end,

    on_mob_kill = function(me, mobs)
        if me == nil or mobs == nil or #mobs == 0 then
            return
        end
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not assert_alive(me) then
            return
        end
        run_nakrang_hunt(me, mobs)
        run_nakrang5_rabbit(me, #mobs)
        run_beginner_path_rabbit(me, #mobs)
    end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end
}
