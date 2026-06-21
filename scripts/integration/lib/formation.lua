local M = {}

local DEFAULT_MAP = "낙랑의방"
local DEFAULT_X   = 6
local DEFAULT_Y   = 6

-- Skill parallel layout (7 bots):
--   direction 1: solo bot at (6, 6)
--   direction 2: causer + target at (13, 6), (14, 6)
--   direction 3: causer + target at (6, 12), (7, 12)
--   direction 4: causer + target at (13, 12), (14, 12)
local SKILL_PARALLEL_LAYOUT = {
    { bots = {0}, positions = {{6, 6}} },
    { bots = {1, 2}, positions = {{13, 6}, {14, 6}} },
    { bots = {3, 4}, positions = {{6, 12}, {7, 12}} },
    { bots = {5, 6}, positions = {{13, 12}, {14, 12}} },
}

M.SKILL_PARALLEL_BOT_COUNT = 7

function M.arrange_in_line(ctx, map_name, start_x, start_y)
    map_name = map_name or DEFAULT_MAP
    start_x  = start_x or DEFAULT_X
    start_y  = start_y or DEFAULT_Y

    local count = ctx:bot_count()
    for i = 0, count - 1 do
        local bot = ctx:bot(i)
        bot:map_move(map_name, start_x + i, start_y)
        bot:direction("BOTTOM")
    end
end

function M.arrange_in_grid(ctx, map_name, start_x, start_y, end_x, end_y)
    local width = end_x - start_x + 1
    local count = ctx:bot_count()
    for i = 0, count - 1 do
        local bot = ctx:bot(i)
        local x   = start_x + (i % width)
        local y   = start_y + math.floor(i / width)
        bot:map_move(map_name, x, y)
    end
end

function M.arrange_skill_parallel(ctx, map_name)
    map_name = map_name or DEFAULT_MAP

    for _, group in ipairs(SKILL_PARALLEL_LAYOUT) do
        for i, bot_index in ipairs(group.bots) do
            local pos = group.positions[i]
            local bot = ctx:bot(bot_index)
            bot:map_move(map_name, pos[1], pos[2])
            bot:direction("BOTTOM")
        end
    end
end

function M.skill_pair_target(causer_index)
    if causer_index == 0 then
        return nil
    end
    if causer_index % 2 == 1 then
        return causer_index + 1
    end
    return nil
end

return M
