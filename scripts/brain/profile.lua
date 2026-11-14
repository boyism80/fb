local M = {}

local records = {}
local running = setmetatable({}, { __mode = "k" })

function M.start(owner)
    records[owner] = { started = clock(), waited = 0, sections = {}, parts = {}, open = {} }
    running[coroutine.running()] = owner
    return records[owner]
end

function M.part(name, fn)
    return function(...)
        local owner  = running[coroutine.running()]
        local record = owner ~= nil and records[owner] or nil
        if record == nil or record.open[name] then
            return fn(...)
        end
        local started      = clock()
        record.open[name]  = true
        local results      = table.pack(pcall(fn, ...))
        record.open[name]  = nil
        record.parts[name] = (record.parts[name] or 0) + clock() - started
        if results[1] == false then
            error(results[2], 0)
        end
        return table.unpack(results, 2, results.n)
    end
end

function M.parts(owner)
    local record = records[owner]
    if record == nil then
        return nil
    end
    local found = {}
    for name, spent in pairs(record.parts) do
        found[name] = spent / 1000
    end
    return found
end

function M.measure(owner, name, fn, ...)
    local record = records[owner]
    if record == nil then
        return fn(...)
    end
    local started         = clock()
    local waited          = record.waited
    local results         = table.pack(fn(...))
    local spent           = clock() - started - (record.waited - waited)
    record.sections[name] = (record.sections[name] or 0) + spent
    return table.unpack(results, 1, results.n)
end

function M.wait(owner, fn, ...)
    local record = records[owner]
    if record == nil or record.waiting then
        return fn(...)
    end
    local started  = clock()
    record.waiting = true
    local results  = table.pack(pcall(fn, ...))
    record.waiting = false
    record.waited  = record.waited + clock() - started
    if results[1] == false then
        error(results[2], 0)
    end
    return table.unpack(results, 2, results.n)
end

function M.spent(owner)
    local record = records[owner]
    if record == nil then
        return nil, nil
    end
    local total = clock() - record.started
    return (total - record.waited) / 1000, record.waited / 1000
end

function M.finish(brain, bot, owner, prefix, started)
    local record = records[owner]
    if record == nil or (started ~= nil and record ~= started) then
        return
    end
    records[owner] = nil
    local name     = prefix or "decide"
    local total    = clock() - record.started
    brain:stat(bot, name .. ":cpu", (total - record.waited) / 1000)
    brain:stat(bot, name .. ":wait", record.waited / 1000)
    for section, spent in pairs(record.sections) do
        brain:stat(bot, name .. ":" .. section, spent / 1000)
    end
end

return M
