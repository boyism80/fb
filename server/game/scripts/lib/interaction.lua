local M = {}

M.chat_regex = {
    SELL = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개)|(?P<all>다|전부))\\s+)?(?:판다|팜|팔게)",
    BUY = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개))\\s+)?(?:산다|줘|주세요)",
    REPAIR = "(((?P<all>전부|모두|다)|(?P<name>\\S+))\\s+?(?:고쳐|수리\\s*해))\\s*줘",
    DEPOSIT_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+맡아\\s*(?:줘|놔|주세요)",
    WITHDRAW_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+돌려\\s*(?:줘|놔|주세요)",
    STORE_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?맡아\\s*(?:줘|놔|주세요)",
    RETRIEVE_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?돌려\\s*(?:줘|놔|주세요)",
    SELL_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:파니|파냐|팔고\\s*(?:있니|있냐))",
    BUY_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:사니|사냐|사고\\s*(?:있니|있냐))",
    SELL_PRICE = "(?P<name>\\S+)\\s+얼마(?:(?:(?:니|야|임|냐|에\\s*파(?:니|냐)))|(?:파(?:니|냐|)))",
    BUY_PRICE = "(?P<name>\\S+)\\s+얼마에\\s?사(?:니|냐)",
    DEPOSITED_MONEY = "(?:돈|금전)\\s*얼마(?:나)?\\s*맡(?:아두)?고\\s*있(?:니|냐)",
    RENAME_WEAPON = "(?P<weapon>\\S+?)?(?:의|$)?\\s+이름을\\s+(?P<name>\\S+?)?(?:으|$)?로\\s+명명",
    HOLD_ITEM_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*맡고\\s*(?:있니|있냐)",
    HOLD_ITEM_COUNT = "(?P<name>\\S+)\\s+(?:몇\\s*개|얼마나)\\s*맡고\\s*있(?:니|냐)",
    REVIVE = "살려(?:(?P<ok>(?:주세요|주십시오))|(?P<no>(?:줘|내|라|주소)))",
    APPRECIATE = "(감사합니다|고맙습니다)",
    JOIN_CASTLE = "참가",
    BLACK_FLAG = "검정깃발",
}

function M.string_split(self, delimiter)
    local result = { }
    local from  = 1
    local delim_from, delim_to = string.find( self, delimiter, from  )
    while delim_from do
        table.insert( result, string.sub( self, from , delim_from-1 ) )
        from  = delim_to + 1
        delim_from, delim_to = string.find( self, delimiter, from  )
    end
    table.insert( result, string.sub( self, from  ) )
    return result
end

function M.is_miss(me, you)
    if debug() then
        return false
    else
        return math.random() > 0.8
    end
end

function M.is_critical(me, you)
    if debug() then
        return true
    else
        return math.random() > 0.8
    end
end

function M.damage(me, you, rate, sound)
    if rate == nil then
        rate = 1.0
    end
    rate = rate * (me:damage_rate() / 1000.0)

    local size = MOB_SIZE.SMALL
    if you:is(OBJECT_TYPE.MOB) then
        local model = you:model()
        size = model:size()
    end

    if me:isbuff('투명') then
        rate = rate * 8
    end

    local critical = M.is_critical(me, you)
    if sound ~= nil then
        you:sound(sound)
    end
    me:damage_to(you, me:normal_attack_damage(size), { critical = critical, rate = rate })
end

function M.any_action(me)
    if me:isbuff('운기') then
        me:unbuff('운기')
    end
end

return M
