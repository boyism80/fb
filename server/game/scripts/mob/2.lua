-- mob: 토끼
-- Rabbit kill count for 낙랑의두루마리5 (q 228); param "squirrel,rabbit", max 5 each.
-- Also increments quest.QUEST_BEGINNER_PATH (사도) hunt param "squirrel,rabbit" at step 5.
local quest = require('lib.quest')

local GOAL = 5

-- @brief   Increment NAKRANG_HUNT progress when killed mob name matches q param.
-- @param[in]  me   The mob that died (rabbit).
-- @param[in]  you  The character who killed the mob.
local function run_nakrang_hunt(me, you)
    local q = you:quest(quest.QUEST_NAKRANG_HUNT)
    if q == nil then
        return
    end
    if me:model():name() ~= q:param() then
        return
    end
    q:inc_progress()
end

-- @brief   Increment rabbit count in quest.QUEST_NAKRANG5 param "squirrel,rabbit" (step 1 only).
-- @param[in]  you  The character who killed the mob.
local function run_nakrang5_rabbit(you)
    local q5 = you:quest(quest.QUEST_NAKRANG5)
    if q5 == nil or q5:completed() or q5:step() ~= 1 then
        return
    end
    local param = q5:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min(tonumber(a) or 0, GOAL)
    local rabbit = math.min((tonumber(b) or 0) + 1, GOAL)
    q5:param(string.format('%d,%d', squirrel, rabbit))
end

-- @brief   Increment rabbit count in quest.QUEST_BEGINNER_PATH param "squirrel,rabbit" (step 5 only, 사도).
-- @param[in]  you  The character who killed the mob.
local function run_beginner_path_rabbit(you)
    local q = you:quest(quest.QUEST_BEGINNER_PATH)
    if q == nil or q:completed() or q:step() ~= 5 then
        return
    end
    local param = q:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min(tonumber(a) or 0, GOAL)
    local rabbit = math.min((tonumber(b) or 0) + 1, GOAL)
    q:param(string.format('%d,%d', squirrel, rabbit))
end

-- @brief   Rabbit mob death: update NAKRANG_HUNT, NAKRANG5, and BEGINNER_PATH (사도) kill counts.
-- @param[in]  me   The rabbit mob that died.
-- @param[in]  you  The character who killed the mob.
function ON_MOB_DIE_2(me, you)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not assert_alive(you) then
        return
    end
    run_nakrang_hunt(me, you)
    run_nakrang5_rabbit(you)
    run_beginner_path_rabbit(you)
end
