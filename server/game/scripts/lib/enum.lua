local M = {}

M.exchange_result = {
    OK = 0,
    LACK_COST = 1,
    LACK_CAPACITY = 2,
}

-- Also registered from C++ as global MOB_PARTS_MODE
M.mob_parts_mode = {
    PARTS = 0,
    BODY = 1,
}

return M
