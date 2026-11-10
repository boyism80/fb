local M = {}

local records = {}

function M.start(owner)
    records[owner] = { started = clock(), waited = 0, sections = {} }
    return records[owner]
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
