local M = {}

M.EXCHANGE_RESULT = {
    OK = 0,
    LACK_COST = 1,
    LACK_CAPACITY = 2,
}

M.RETURN_MAP_DATA = {
    [NATION.GOGURYEO] = {
        maps = {29, 30, 31, 8016, 8017, 8018, 8043, 8044, 8045, 8070, 8071, 8072, 8097, 8098, 8099, 8124, 8125, 8126, 8151, 8152, 8153, 8178, 8179, 8180, 8205, 8206, 8207, 8232, 8233, 8234, 8259, 8260, 8261},
    },
    [NATION.BUYEO] = {
        maps = {15, 16, 17, 7716, 7717, 7718, 7743, 7744, 7745, 7770, 7771, 7772, 7797, 7798, 7799, 7824, 7825, 7826, 7851, 7852, 7853, 7878, 7879, 7880, 7905, 7906, 7907, 7932, 7933, 7934, 7959, 7960, 7961},
    },
}

function M.warp_to_return_map(ch)
    local data = M.RETURN_MAP_DATA[ch:nation()]
    if data == nil then
        return nil
    end
    local map = data.maps[math.random(1, #data.maps)]
    return ch:map(map)
end

return M
