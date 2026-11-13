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

local ESCAPES = { b = "\b", f = "\f", n = "\n", r = "\r", t = "\t" }

local decode = nil

local function text_at(source, at)
    local parts = {}
    local from  = at + 1
    while true do
        local stop = source:find('["\\]', from)
        assert(stop ~= nil, "json: unterminated string")
        table.insert(parts, source:sub(from, stop - 1))
        if source:sub(stop, stop) == '"' then
            return table.concat(parts), stop + 1
        end
        local mark = source:sub(stop + 1, stop + 1)
        if mark == "u" then
            local code = tonumber(source:sub(stop + 2, stop + 5), 16)
            from       = stop + 6
            if code >= 0xD800 and code < 0xDC00 and source:sub(from, from + 1) == "\\u" then
                local low = tonumber(source:sub(from + 2, from + 5), 16)
                code      = 0x10000 + (code - 0xD800) * 0x400 + (low - 0xDC00)
                from      = from + 6
            end
            table.insert(parts, utf8.char(code))
        else
            table.insert(parts, ESCAPES[mark] or mark)
            from = stop + 2
        end
    end
end

decode = function(source, at)
    at         = assert(source:find("%S", at), "json: unexpected end")
    local mark = source:sub(at, at)
    if mark == "{" or mark == "[" then
        local found = {}
        local close = mark == "{" and "}" or "]"
        at          = source:find("%S", at + 1)
        if source:sub(at, at) == close then
            return found, at + 1
        end
        while true do
            local key = #found + 1
            if mark == "{" then
                key, at = decode(source, at)
                at      = assert(source:find(":", at, true), "json: missing colon") + 1
            end
            found[key], at = decode(source, at)
            at             = assert(source:find("%S", at), "json: unexpected end")
            local between  = source:sub(at, at)
            at             = at + 1
            if between == close then
                return found, at
            end
            assert(between == ",", "json: missing comma")
        end
    elseif mark == '"' then
        return text_at(source, at)
    elseif source:sub(at, at + 3) == "true" then
        return true, at + 4
    elseif source:sub(at, at + 4) == "false" then
        return false, at + 5
    elseif source:sub(at, at + 3) == "null" then
        return nil, at + 4
    else
        local number = assert(source:match("^-?%d+%.?%d*[eE]?[-+]?%d*", at), "json: unexpected character")
        return math.tointeger(tonumber(number)) or tonumber(number), at + #number
    end
end

function M.decode(source)
    return (decode(source, 1))
end

function M.read(path)
    local file = io.open(path, "rb")
    if file == nil then
        return nil
    end
    local source = file:read("a")
    file:close()
    return M.decode(source)
end

function M.write(path, value)
    local file = assert(io.open(path, "wb"))
    file:write(M.encode(value), "\n")
    file:close()
end

return M
