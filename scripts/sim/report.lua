local json  = require("sim.world.json")
local lanes = require("sim.world.lanes")

local M = {}

M.SHOWN     = 30
local LANES = 99

local function rows(path, mark)
    local found = {}
    local file  = io.open(path, "rb")
    if file == nil then
        return found
    end
    for line in file:lines() do
        if mark == nil or line:find(mark, 1, true) ~= nil then
            local ok, row = pcall(json.decode, line)
            if ok and type(row) == "table" then
                table.insert(found, row)
            end
        end
    end
    file:close()
    return found
end

local function count(path)
    local total = 0
    local file  = io.open(path, "rb")
    if file ~= nil then
        for _ in file:lines() do
            total = total + 1
        end
        file:close()
    end
    return total
end

local function added(into, from)
    for name, entry in pairs(from or {}) do
        local known = into[name]
        if known == nil then
            known = {}
            into[name] = known
        end
        for field, value in pairs(entry) do
            if type(value) == "number" then
                known[field] = (known[field] or 0) + value
            elseif type(value) == "table" then
                known[field] = known[field] or {}
                for key, part in pairs(value) do
                    known[field][key] = (known[field][key] or 0) + part
                end
            end
        end
    end
end

function M.collect(home)
    local asked   = (json.read(home .. "/params.json") or {}).sim or {}
    local costs   = json.read(lanes.COSTS) or {}
    local files   = {}
    local result  = {
        run = home:match("([^/\\]+)$"),
        lanes = 0,
        jobs = 0,
        job_seconds = 0,
        decisions = 0,
        decide_seconds = 0,
        steps = 0,
        teams = {},
        solos = {},
        errors = {},
        rules = {},
        violations = {},
        broke = {},
        stopped = false,
    }
    local profile = nil
    for lane = 1, LANES do
        local found = json.read(string.format("%s/lane-%02d.json", home, lane))
        if found ~= nil then
            result.lanes          = result.lanes + 1
            result.decisions      = result.decisions + (found.decisions or 0)
            result.decide_seconds = result.decide_seconds + (found.decide or 0)
            result.steps          = result.steps + (found.steps or 0)
            for _, job in ipairs(found.jobs or {}) do
                result.jobs        = result.jobs + 1
                result.job_seconds = result.job_seconds + job.seconds
                costs[job.key]     = job.seconds
                for _, bot in ipairs(job.bots or {}) do
                    bot.seed = job.seed
                    if bot.file ~= nil then
                        files[bot.file] = true
                    end
                    if bot.failed ~= nil then
                        bot.team, bot.profile = job.team, job.profile
                        table.insert(result.errors, bot)
                    elseif job.team ~= nil then
                        bot.team = job.team
                        table.insert(result.teams, bot)
                    else
                        bot.profile = job.profile
                        table.insert(result.solos, bot)
                    end
                end
            end
            if found.profile ~= nil then
                profile      = profile or { wall = 0, functions = {}, sites = {}, callers = {}, choosers = {} }
                profile.wall = profile.wall + (found.profile.wall or 0)
                for _, part in ipairs({ "functions", "sites", "callers", "choosers" }) do
                    added(profile[part], found.profile[part])
                end
            end
        end
    end
    result.profile = profile
    result.ms      = result.decisions > 0 and result.decide_seconds / result.decisions * 1000 or 0

    local alarm = home .. "/alarm.jsonl"
    for _, row in ipairs(rows(alarm)) do
        local rule         = tostring(row.rule)
        result.rules[rule] = (result.rules[rule] or 0) + 1
        if #result.violations < M.SHOWN then
            table.insert(result.violations, row)
        end
    end
    result.audit = count(alarm .. ".audit.txt")

    for path in pairs(files) do
        for _, row in ipairs(rows(path, '"kind":"broke"')) do
            local seconds = ((row.to or 0) - (row.from or 0)) / 1000
            local known   = result.broke[row.rule]
            if known == nil then
                known = { breaks = 0, seconds = 0, longest = 0 }
                result.broke[row.rule] = known
            end
            known.breaks  = known.breaks + 1
            known.seconds = known.seconds + seconds
            known.longest = math.max(known.longest, seconds)
        end
    end

    local halt = io.open(home .. "/stop", "rb")
    if halt ~= nil then
        halt:close()
        result.stopped = true
    end
    if asked.corpus then
        result.corpus = home .. "/corpus"
    end
    json.write(lanes.COSTS, costs)
    json.write(home .. "/result.json", result)
    return result
end

return M
