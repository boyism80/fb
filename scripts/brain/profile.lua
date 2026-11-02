local M = {}

local records = {}

function M.start(owner)
    records[owner] = { started = clock(), waited = 0, sections = {} }
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
    if record == nil then
        return fn(...)
    end
    local started = clock()
    local results = table.pack(fn(...))
    record.waited = record.waited + clock() - started
    return table.unpack(results, 1, results.n)
end

function M.finish(brain, bot, owner)
    local record = records[owner]
    records[owner] = nil
    if record == nil then
        return
    end
    local total = clock() - record.started
    brain:stat(bot, "decide:cpu", (total - record.waited) / 1000)
    brain:stat(bot, "decide:wait", record.waited / 1000)
    for name, spent in pairs(record.sections) do
        brain:stat(bot, "decide:" .. name, spent / 1000)
    end
end

return M
