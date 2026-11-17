local WALK_MS = require("brain.data.combat").walk

local function wait_sec(period, dock)
    local gap = period - dock
    if period <= 0 or gap <= 0 then
        return 0
    end
    return (gap * gap) / (2 * period)
end

local function steps_of(seconds)
    return math.ceil(seconds * 1000 / WALK_MS)
end

local ROUTES = {
    {
        id = "buyeo_gogundo",
        min_level = 21,
        ticket = "부여-고균도승선권",
        ticket_price = 100,
        seller = "부여고균도매표상",
        sailor = "부여-섬선원",
        boatman = "부여-섬뱃사공",
        board = { map = 1744, x = 17, y = 36 },
        cabin = { map = 1745, x = 8, y = 7 },
        land = { map = 1744, x = 14, y = 240 },
        return_land = { map = 1744, x = 14, y = 26 },
        period = 7200,
        dock = 1200,
        sail = 1800,
        board_menu = { "배를 태워주세요.", "네, 승선시켜 주시기 바랍니다." },
        docked = "배가 선착장에 정박해 있습니다.",
        boarding = "그럼, 선실에 승선시켜드리도록 하겠습니다.",
        island = "고균도입니다. 안녕히 가십시오.",
        home = "부여입니다. 안녕히 가십시오.",
    },
    {
        id = "goguryeo_gogundo",
        min_level = 21,
        ticket = "고구려-고균도승선권",
        ticket_price = 100,
        seller = "고구려고균도매표상",
        sailor = "고구려-섬선원",
        boatman = "고구려-섬뱃사공",
        board = { map = 1746, x = 17, y = 36 },
        cabin = { map = 1747, x = 8, y = 7 },
        land = { map = 1746, x = 14, y = 240 },
        return_land = { map = 1746, x = 14, y = 26 },
        period = 7200,
        dock = 1200,
        sail = 1800,
        board_menu = { "배를 태워주세요.", "네, 승선시켜 주시기 바랍니다." },
        docked = "배가 선착장에 정박해 있습니다.",
        boarding = "그럼, 선실에 승선시켜드리도록 하겠습니다.",
        island = "고균도입니다. 안녕히 가십시오.",
        home = "고구려입니다. 안녕히 가십시오.",
    },
}

for _, route in ipairs(ROUTES) do
    route.wait = wait_sec(route.period, route.dock)
    route.cost = steps_of(route.wait + route.sail)
end

return {
    routes = ROUTES,
    walk = WALK_MS,
}
