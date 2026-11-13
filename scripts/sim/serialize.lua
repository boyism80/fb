local M = {}

local function number(value)
    if math.type(value) == "integer" then
        return tostring(value)
    elseif value ~= value then
        return "0/0"
    elseif value == math.huge then
        return "math.huge"
    elseif value == -math.huge then
        return "-math.huge"
    elseif value == math.floor(value) and math.abs(value) < 2 ^ 53 then
        return string.format("%.1f", value)
    end
    return string.format("%.17g", value)
end

local function encode(value, out)
    local kind = type(value)
    if kind == "table" then
        table.insert(out, "{ ")
        local keys = {}
        for key in pairs(value) do
            table.insert(keys, key)
        end
        table.sort(keys, function(left, right)
            return tostring(left) < tostring(right)
        end)
        for _, key in ipairs(keys) do
            if type(key) == "string" and key:match("^[%a_][%w_]*$") then
                table.insert(out, key .. " = ")
            else
                table.insert(out, "[")
                encode(key, out)
                table.insert(out, "] = ")
            end
            encode(value[key], out)
            table.insert(out, ", ")
        end
        table.insert(out, "}")
    elseif kind == "string" then
        table.insert(out, (string.gsub(string.format("%q", value), "\\\n", "\\n")))
    elseif kind == "number" then
        table.insert(out, number(value))
    else
        table.insert(out, tostring(value))
    end
end

local function plain(value, seen)
    local kind = type(value)
    if kind == "number" or kind == "string" or kind == "boolean" then
        return value
    end
    if kind ~= "table" or seen[value] then
        return nil
    end
    seen[value] = true
    local copied = {}
    for key, entry in pairs(value) do
        if type(key) == "number" or type(key) == "string" then
            copied[key] = plain(entry, seen)
        end
    end
    seen[value] = nil
    return copied
end

function M.frozen(value)
    return plain(value, {})
end

function M.text(value)
    local out = { "return " }
    encode(value, out)
    return table.concat(out)
end

function M.dump(path, value)
    return M.write(path, plain(value, {}))
end

function M.write(path, value)
    local file = io.open(path, "w")
    if file == nil then
        return false
    end
    file:write(M.text(value), "\n")
    file:close()
    return true
end

return M
