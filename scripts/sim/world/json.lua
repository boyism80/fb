local M = {}

local function array(value)
    local count = 0
    for _ in pairs(value) do
        count = count + 1
    end
    return count > 0 and count == #value
end

local function encode(value, out)
    local kind = type(value)
    if kind == "table" then
        if array(value) then
            table.insert(out, "[")
            for index, entry in ipairs(value) do
                if index > 1 then
                    table.insert(out, ",")
                end
                encode(entry, out)
            end
            table.insert(out, "]")
        else
            table.insert(out, "{")
            local first = true
            for key, entry in pairs(value) do
                if type(entry) ~= "function" then
                    if first == false then
                        table.insert(out, ",")
                    end
                    first = false
                    encode(tostring(key), out)
                    table.insert(out, ":")
                    encode(entry, out)
                end
            end
            table.insert(out, "}")
        end
    elseif kind == "string" then
        table.insert(out, '"' .. value:gsub('[%c"\\]', function(char)
            return string.format("\\u%04x", char:byte())
        end) .. '"')
    elseif kind == "number" then
        if value ~= value or value == math.huge or value == -math.huge then
            table.insert(out, "null")
        elseif math.type(value) == "integer" then
            table.insert(out, tostring(value))
        else
            table.insert(out, string.format("%.6g", value))
        end
    elseif kind == "boolean" then
        table.insert(out, tostring(value))
    else
        table.insert(out, "null")
    end
end

function M.encode(value)
    local out = {}
    encode(value, out)
    return table.concat(out)
end

return M
