-- mob: 다람쥐
-- Squirrel kill count for 낙랑의두루마리5 (q 228); param "squirrel,rabbit", max 5 each.
-- Also increments quest.QUEST_BEGINNER_PATH (사도) hunt param "squirrel,rabbit" at step 5.
local quest = require('lib.quest')

local GOAL = 5

-- @brief   Increment NAKRANG_HUNT progress when killed mob name matches q param.
-- @param[in]  me    The character who killed.
-- @param[in]  mobs  Killed squirrel mobs.
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

-- @brief   Increment squirrel count in quest.QUEST_NAKRANG5 param "squirrel,rabbit" (step 1 only).
local function run_nakrang5_squirrel(me, count)
    local q5 = me:quest(quest.QUEST_NAKRANG5)
    if q5 == nil or q5:completed() or q5:step() ~= 1 then
        return
    end
    local param = q5:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min((tonumber(a) or 0) + count, GOAL)
    local rabbit = math.min(tonumber(b) or 0, GOAL)
    q5:param(string.format('%d,%d', squirrel, rabbit))
end

-- @brief   Increment squirrel count in quest.QUEST_BEGINNER_PATH param "squirrel,rabbit" (step 5 only, 사도).
local function run_beginner_path_squirrel(me, count)
    local q = me:quest(quest.QUEST_BEGINNER_PATH)
    if q == nil or q:completed() or q:step() ~= 5 then
        return
    end
    local param = q:param() or '0,0'
    local a, b = param:match('^(%d+),(%d+)$')
    if not a or not b then
        return
    end
    local squirrel = math.min((tonumber(a) or 0) + count, GOAL)
    local rabbit = math.min(tonumber(b) or 0, GOAL)
    q:param(string.format('%d,%d', squirrel, rabbit))
end

return {
    -- ON_MOB_ATTACK = function(me, you)
    -- end,

    -- ON_MOB_DIE = function(me)
    -- end,

    ON_MOB_KILL = function(me, mobs)
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
        run_nakrang5_squirrel(me, #mobs)
        run_beginner_path_squirrel(me, #mobs)
    end,

    -- ON_MOB_SPELL_HIT = function(me, you, spell)
    -- end
}
