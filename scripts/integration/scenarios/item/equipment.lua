local item_lib = require("integration.lib.item")
local model    = require("integration.model")

local EQUIPMENT_PART_BY_TYPE = item_lib.EQUIPMENT_PART_BY_TYPE

function run(ctx, index)
    local bot       = ctx:bot(index)
    local bot_count = ctx:bot_count()
    local seq       = 0

    for _, entry in ipairs(model.equipment_items()) do
        if seq % bot_count == index then
            bot:create_item(entry.name, 1)

            local equipped = false
            local part     = EQUIPMENT_PART_BY_TYPE[entry.type]

            if entry.has_condition == false then
                if bot:equip(0) == false then
                    log("fatal", string.format("[%d] Equip failed: %s (no condition)", seq + 1, entry.name))
                    return false
                end
                equipped = true
            else
                bot:reverse_item_condition(entry.name)
                if bot:equip(0) then
                    log("debug", string.format("[%d] Unexpectedly equipped: %s (condition reversed)", seq + 1, entry.name))
                    return false
                end

                bot:apply_item_condition(entry.name)
                if bot:equip(0) == false then
                    log("fatal", string.format("[%d] Equip failed: %s (condition applied)", seq + 1, entry.name))
                    return false
                end
                equipped = true

                if part == nil then
                    log("debug", string.format("[%d] Equip part not found: %s", seq + 1, entry.name))
                    return false
                end
            end

            if equipped and part ~= nil then
                bot:unequip(part)
            end

            bot:clear_inventory()
        end
        seq = seq + 1
    end

    return true
end

return { run = run }
