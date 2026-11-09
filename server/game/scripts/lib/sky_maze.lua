local M = {}

local SKY_MAZE_NAMES = {
    "천상미궁시작", "천상미궁1", "천상미궁2", "천상미궁3", "천상미궁4", "천상미궁5",
    "천상미궁6", "천상미궁7", "천상미궁8", "천상미궁9", "천상미궁10", "천상미궁11",
    "천상미궁12", "천상미궁13", "천상미궁14", "천상미궁15", "천상미궁16", "천상미궁17",
    "천상미궁18", "천상미궁19", "천상미궁20", "천상미궁21", "천상미궁22", "천상미궁23",
    "천상미궁24", "천상미궁25", "천상미궁비밀방",
}
local PK_SKY_MAZE_NAMES = {
    "PK천상미궁시작", "PK천상미궁1", "PK천상미궁2", "PK천상미궁3", "PK천상미궁4", "PK천상미궁5",
    "PK천상미궁6", "PK천상미궁7", "PK천상미궁8", "PK천상미궁9", "PK천상미궁10", "PK천상미궁11",
    "PK천상미궁12", "PK천상미궁13", "PK천상미궁14", "PK천상미궁15", "PK천상미궁16", "PK천상미궁17",
    "PK천상미궁18", "PK천상미궁19", "PK천상미궁20", "PK천상미궁21", "PK천상미궁22", "PK천상미궁23",
    "PK천상미궁24", "PK천상미궁25", "PK천상미궁비밀방",
}

M.NORMAL = { names = SKY_MAZE_NAMES, prefix = "sky_maze_", entrance = 2996, exit = 649 }
M.PK = { names = PK_SKY_MAZE_NAMES, prefix = "pk_sky_maze_", entrance = 2479, exit = 2474 }

local function map_name_to_id(name)
    local m = name2map(name)
    if m ~= nil then return m:model():id() end
    return nil
end

local function shuffle_array(arr, from_idx, to_idx)
    for _ = 1, (to_idx - from_idx + 1) do
        local i = math.random(from_idx, to_idx)
        local j = math.random(from_idx, to_idx)
        arr[i], arr[j] = arr[j], arr[i]
    end
end

function M.shuffle(maze, value)
    math.randomseed(value or seed())
    local arr = {}
    for i = 0, 26 do
        arr[i] = map_name_to_id(maze.names[i + 1])
    end
    shuffle_array(arr, 1, 25)
    for i = 0, 26 do
        property(maze.prefix .. tostring(i), arr[i])
    end
end

local function slot_ids(maze)
    if property(maze.prefix .. "0") == nil then
        M.shuffle(maze)
    end

    local arr = {}
    for i = 0, 26 do
        arr[i] = property(maze.prefix .. tostring(i)) or map_name_to_id(maze.names[i + 1])
    end
    return arr
end

local function slot_position(slots, map_id)
    for i = 1, 26 do
        if slots[i] == map_id then
            return i
        end
    end
    return nil
end

local function slot_name(maze, slots, i)
    local m = id2map(slots[i])
    if m ~= nil then
        return m:model():name()
    else
        return maze.names[i + 1]
    end
end

local function sky_maze_next(my_pos, side)
    if side == 0 then
        local next_pos = my_pos - 5
        if next_pos <= 0 then
            next_pos = next_pos + 25
        end
        return next_pos
    elseif side == 1 then
        if my_pos % 5 == 0 then
            return my_pos - 4
        else
            return my_pos + 1
        end
    elseif side == 2 then
        local next_pos = my_pos + 5
        if next_pos > 25 then
            next_pos = next_pos - 25
        end
        return next_pos
    else
        local next_pos = my_pos - 1
        if next_pos % 5 == 0 then
            next_pos = next_pos + 5
        end
        return next_pos
    end
end

function M.warp(me, maze)
    if me == nil then
        return
    end
    local map = me:map()
    if map == nil then
        return
    end
    local x, y = me:position()

    local side
    if x == 24 and y == 1 then
        side = 0
    elseif x == 47 and y == 24 then
        side = 1
    elseif x == 24 and y == 47 then
        side = 2
    elseif x == 1 and y == 24 then
        side = 3
    else
        return
    end

    local slots = slot_ids(maze)
    local map_id = map:model():id()
    local pos = slot_position(slots, map_id)
    local dest = nil
    local dest_x, dest_y = 24, 24
    if map_id == maze.entrance then
        dest = slots[math.random(1, 24)]
    elseif pos == 26 then
        dest = maze.exit
        dest_x, dest_y = 1, 1
    elseif pos == 25 then
        dest = slots[26]
    elseif pos ~= nil then
        dest = slots[sky_maze_next(pos, side)]
    end

    local dest_map = dest ~= nil and id2map(dest) or nil
    if dest_map ~= nil then
        me:map(dest_map, dest_x, dest_y)
    end
end

local function sky_maze_neighbors(pos)
    local out = {}
    for side = 0, 3 do
        out[#out + 1] = sky_maze_next(pos, side)
    end
    return out
end

local function find_secret_route(start_pos)
    local goal_pos = 25
    local parent = {}
    parent[start_pos] = -1
    local queue = { start_pos }
    local head = 1
    while head <= #queue do
        local cur = queue[head]
        head = head + 1
        if cur == goal_pos then
            break
        end
        local neighbors = sky_maze_neighbors(cur)
        for i = 1, #neighbors do
            local next_pos = neighbors[i]
            if parent[next_pos] == nil then
                parent[next_pos] = cur
                queue[#queue + 1] = next_pos
            end
        end
    end
    local path = {}
    local p = goal_pos
    while p ~= -1 do
        path[#path + 1] = p
        p = parent[p]
    end
    local rev = {}
    for i = #path, 1, -1 do
        rev[#rev + 1] = path[i]
    end
    return rev
end

function M.print_route(me, maze)
    local map = me:map()
    if map == nil then
        return
    end

    local slots = slot_ids(maze)
    local pos = slot_position(slots, map:model():id())
    if pos == nil or pos == 26 then
        me:message(maze.names[2] .. "~" .. maze.names[26] .. " 칸에서 사용하세요.", MESSAGE_TYPE.BROWN)
    else
        local route = find_secret_route(pos)
        route[#route + 1] = 26

        local dir_names = { "북", "동", "남", "서" }
        for i = 1, #route - 1 do
            local dir_str = ""
            if route[i + 1] <= 25 then
                for side = 0, 3 do
                    if sky_maze_next(route[i], side) == route[i + 1] then
                        dir_str = " (" .. dir_names[side + 1] .. ")"
                        break
                    end
                end
            end
            me:message(slot_name(maze, slots, route[i]) .. dir_str .. " - " .. slot_name(maze, slots, route[i + 1]),
                MESSAGE_TYPE.BROWN)
        end
    end
end

return M
