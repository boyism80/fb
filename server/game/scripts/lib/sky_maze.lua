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

local function map_name_to_id(name)
    local m = name2map(name)
    if m ~= nil then return m:id() end
    return nil
end

local SKY_MAZE_ENTRANCE_MAP_ID = 2996
local SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID = 2700
local SKY_MAZE_FINAL_EXIT_MAP_ID = 649
local PK_SKY_MAZE_ENTRANCE_MAP_ID = 2479
local PK_SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID = 2750
local PK_SKY_MAZE_FINAL_EXIT_MAP_ID = 2474

local function shuffle_array(arr, from_idx, to_idx)
    for _ = 1, (to_idx - from_idx + 1) do
        local i = math.random(from_idx, to_idx)
        local j = math.random(from_idx, to_idx)
        arr[i], arr[j] = arr[j], arr[i]
    end
end

function M.shuffle(seed)
    math.randomseed(seed or now() or os.time())
    local arr = {}
    for i = 0, 26 do
        arr[i] = map_name_to_id(SKY_MAZE_NAMES[i + 1])
    end
    shuffle_array(arr, 1, 25)
    for i = 0, 26 do
        property("sky_maze_" .. tostring(i), arr[i])
    end
end

function M.pk_shuffle(seed)
    math.randomseed(seed or now() or os.time())
    local arr = {}
    for i = 0, 26 do
        arr[i] = map_name_to_id(PK_SKY_MAZE_NAMES[i + 1])
    end
    shuffle_array(arr, 1, 25)
    for i = 0, 26 do
        property("pk_sky_maze_" .. tostring(i), arr[i])
    end
end

function M.get_slot(i)
    local map_id = property("sky_maze_" .. tostring(i))
    if map_id == nil then
        return SKY_MAZE_NAMES[i + 1]
    end
    local m = id2map(map_id)
    if m ~= nil then
        return m:name()
    end
    return SKY_MAZE_NAMES[i + 1]
end

function M.get_pk_slot(i)
    local map_id = property("pk_sky_maze_" .. tostring(i))
    if map_id == nil then
        return PK_SKY_MAZE_NAMES[i + 1]
    end
    local m = id2map(map_id)
    if m ~= nil then
        return m:name()
    end
    return PK_SKY_MAZE_NAMES[i + 1]
end

local function get_sky_maze_slot_id(i)
    local map_id = property("sky_maze_" .. tostring(i))
    if map_id == nil then
        return map_name_to_id(SKY_MAZE_NAMES[i + 1])
    end
    return map_id
end

local function get_pk_sky_maze_slot_id(i)
    local map_id = property("pk_sky_maze_" .. tostring(i))
    if map_id == nil then
        return map_name_to_id(PK_SKY_MAZE_NAMES[i + 1])
    end
    return map_id
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

function M.warp(me)
    if me == nil then 
        return
    end
    local map = me:map()
    if map == nil then 
        return
    end
    local map_id = map:model():id()
    local map_name = map:model():name()
    local x, y = me:position()
    
    if property("sky_maze_0") == nil then
        M.shuffle()
    end
    
    local arr = {}
    for i = 0, 26 do
        arr[i] = get_sky_maze_slot_id(i) 
    end
    
    local my_pos = nil
    if map_id == SKY_MAZE_ENTRANCE_MAP_ID then
        my_pos = math.random(1, 25)
    else
        for i = 1, 25 do
            if map_id == arr[i] then 
                my_pos = i 
                break
            end
        end
    end
    
    if my_pos == nil then
        return
    end
    
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
    
    if map_id == arr[26] then
        local m = id2map(SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID)
        if m then 
            me:map(m, 24, 24) 
        end
        return
    end
    if map_id == SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID then
        local m = id2map(SKY_MAZE_FINAL_EXIT_MAP_ID)
        if m then 
            me:map(m, 1, 1) 
        end
        return
    end
    
    -- From grid position 25, any of the 4 warp tiles leads to secret room (slot 26)
    if my_pos == 25 then
        local secret = id2map(arr[26])
        if secret then
            me:map(secret, 24, 24)
        end
        return
    end
    
    local next_pos = sky_maze_next(my_pos, side)
    local dest = id2map(arr[next_pos])
    if dest then 
        me:map(dest, 24, 24) 
    end
end

function M.warp_pk(me)
    if me == nil then 
        return 
    end
    local map = me:map()
    if map == nil then 
        return 
    end
    local map_id = map:model():id()
    local map_name = map:model():name()
    local x, y = me:position()
    
    if property("pk_sky_maze_0") == nil then
        M.pk_shuffle()
    end
    
    local arr = {}
    for i = 0, 26 do 
        arr[i] = get_pk_sky_maze_slot_id(i) 
    end
    
    local my_pos = nil
    if map_id == PK_SKY_MAZE_ENTRANCE_MAP_ID then
        my_pos = math.random(1, 25)
    else
        for i = 1, 25 do
            if map_id == arr[i] then 
                my_pos = i 
                break
            end
        end
    end
    
    if my_pos == nil then 
        return 
    end
    
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
    
    if map_id == arr[26] then
        local m = id2map(PK_SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID)
        if m then 
            me:map(m, 24, 24) 
        end
        return
    end
    if map_id == PK_SKY_MAZE_EXIT_AFTER_SECRET_MAP_ID then
        local m = id2map(PK_SKY_MAZE_FINAL_EXIT_MAP_ID)
        if m then 
            me:map(m, 1, 1) 
        end
        return
    end
    
    -- From grid position 25, any of the 4 warp tiles leads to secret room (slot 26)
    if my_pos == 25 then
        local secret = id2map(arr[26])
        if secret then
            me:map(secret, 24, 24)
        end
        return
    end
    
    local next_pos = sky_maze_next(my_pos, side)
    local dest = id2map(arr[next_pos])
    if dest then 
        me:map(dest, 24, 24) 
    end
end

local function sky_maze_neighbors(pos)
    local out = {}
    for side = 0, 3 do
        out[#out + 1] = sky_maze_next(pos, side)
    end
    return out
end

local function sky_maze_find_route_to_secret()
    local start_pos, goal_pos = 1, 25
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
    if parent[goal_pos] == nil then
        return {}
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

function M.print_route(me)
    if property("sky_maze_0") == nil then
        M.shuffle()
    end

    local path = sky_maze_find_route_to_secret()
    if #path == 0 then
        me:message("No route to secret room (unreachable).", MESSAGE_TYPE.BROWN)
        return
    end

    local route = { 0 }
    for i = 1, #path do
        route[#route + 1] = path[i]
    end
    route[#route + 1] = 26

    local dir_names = { "북", "동", "남", "서" }
    for i = 1, #route - 1 do
        local a = M.get_slot(route[i])
        local b = M.get_slot(route[i + 1])
        local dir_str = ""
        if route[i] >= 1 and route[i] <= 25 and route[i + 1] >= 1 and route[i + 1] <= 25 then
            for side = 0, 3 do
                if sky_maze_next(route[i], side) == route[i + 1] then
                    dir_str = " (" .. dir_names[side + 1] .. ")"
                    break
                end
            end
        end
        me:message((a or "?") .. dir_str .. " - " .. (b or "?"), MESSAGE_TYPE.BROWN)
    end
end

return M
