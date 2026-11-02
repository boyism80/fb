local M = {}

local built = {}

function M.combat()
    if built.combat == nil then
        built.combat = {
            kind = NODE.SELECTOR,
            children = {
                { kind = NODE.CHECK },
                { kind = NODE.SELECTOR, children = { { kind = NODE.HEAL }, { kind = NODE.POTION }, { kind = NODE.HURT } } },
                { kind = NODE.BUFF },
                { kind = NODE.SPELL },
                { kind = NODE.MANA },
                {
                    kind = NODE.SEQUENCE,
                    children = {
                        { kind = NODE.APPROACH },
                        { kind = NODE.FACE },
                        { kind = NODE.SELECTOR, children = { { kind = NODE.OPENER }, { kind = NODE.SWING } } },
                    },
                },
            },
        }
    end
    return built.combat
end

return M
