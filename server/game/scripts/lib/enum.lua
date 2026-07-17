local M = {}

M.EXCHANGE_RESULT = {
    OK = 0,
    LACK_COST = 1,
    LACK_CAPACITY = 2,
}

-- Also registered from C++ as global MOB_PARTS_MODE
M.MOB_PARTS_MODE = {
    PARTS = 0,
    BODY = 1,
}

return M
