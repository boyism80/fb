local M = {}

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
        table.insert(out, string.format("%q", value))
    else
        table.insert(out, tostring(value))
    end
end

function M.write(path, value)
    local out = { "return " }
    encode(value, out)
    local file = io.open(path, "w")
    if file == nil then
        return false
    end
    file:write(table.concat(out), "\n")
    file:close()
    return true
end

return M
