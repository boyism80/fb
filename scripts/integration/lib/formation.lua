local M = {}

local DEFAULT_MAP  = "낙랑의방"
local DEFAULT_X    = 6
local DEFAULT_Y    = 6

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

return M
