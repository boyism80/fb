function string:split(delimiter)
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

command_funcs = {
    ['관리자'] =
    function (me, args)
        me:mkspell('강제이동(좌)')
        me:mkspell('강제이동(하)')
        me:mkspell('강제이동(우)')
        me:mkspell('강제이동(상)')
        return true
    end,

    ['경험치'] =
    function (me, args)
        local exp = table.unpack(args)
        exp = tonumber(exp)
        me:exp(exp)
        return true
    end,

    ['맵이동'] =
    function (me, args)
        local map, x, y = table.unpack(args)
        if name2map(map) == nil then
            return false
        end

        local error = false
        if x ~= nil and y ~= nil then
            error = me:map(map, x, y)
        else
            error = me:map(map)
        end
        return true
    end,

    ['사운드'] =
    function (me, args)
        local sound = table.unpack(args)
        me:sound(sound)
        return true
    end,

    ['액션'] = 
    function (me, args)
        local action = table.unpack(args)
        me:action(action)
        return true
    end,

    ['날씨'] = 
    function (me, args)
        local value = table.unpack(args)
        weather(value)
        return true
    end,

    ['밝기'] = 
    function (me, args)
        local value = table.unpack(args)
        bright(value)
        return true
    end,

    ['타이머'] = 
    function (me, args)
        local time = table.unpack(args)
        timer(time, true)
        return true
    end,

    ['타이틀'] = 
    function (me, args)
        local value = table.unpack(args)
        me:title(value)
        return true
    end,

    ['이펙트'] =
    function (me, args)
        local value = table.unpack(args)
        me:effect(value)
        return true
    end,

    ['변신'] = 
    function (me, args)
        local value = table.unpack(args)
        me:disguise(tonumber(value))
        return true
    end,

    ['변신해제'] = 
    function (me, args)
        me:disguise(nil)
        return true
    end,

    ['마법배우기'] =
    function (me, args)
        local name = table.unpack(args)
        me:mkspell(name)
        return true
    end,

    ['마법지우기'] = 
    function (me, args)
        if #args == 0 then
            me:rmspell()
        else
            local slot = table.unpack(args)
            me:rmspell(slot)
        end
        return true
    end,

    ['몬스터생성'] = 
    function (me, args)
        local name = table.unpack(args)
        local x, y = me:position()
        me:spawn_mob(name, x, y, false)
        return true
    end,

    ['직업바꾸기'] = 
    function (me, args)
        local name = table.unpack(args)
        local class, promotion = name2class(name)
        if class ~= nil then
            me:class(class)
            me:promotion(promotion)
        end
        return true
    end,

    ['레벨바꾸기'] = 
    function (me, args)
        local level = table.unpack(args)
        me:level(level)
        return true
    end,

    ['힘바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_str(value)
        return true
    end,

    ['민첩바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_dex(value)
        return true
    end,

    ['지력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_int(value)
        return true
    end,

    ['체력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_hp(value)
        me:hp(me:maxhp())
        if me:state() == STATE_GHOST then
            me:state(STATE_NORMAL)
        end
        return true
    end,

    ['마력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_mp(value)
        me:mp(me:maxmp())
        return true
    end,

    ['아이템생성'] = 
    function (me, args)
        local name, count = table.unpack(args)
        me:mkitem(name, count)
        return true
    end,

    ['월드맵'] = 
    function (me, args)
        local name = table.unpack(args)
        me:world(name)
        return true
    end,

    ['스크립트'] = 
    function (me, args)
        me:script()
        return true
    end,

    ['머리바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:look(value)
        return true
    end,

    ['머리염색'] = 
    function (me, args)
        local value = table.unpack(args)
        me:color(value)
        return true
    end,

    ['갑옷염색'] = 
    function (me, args)
        local value = table.unpack(args)
        me:armor_color(value)
        return true
    end,

    ['서버종료'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['맵타일'] = 
    function (me, args)
        local map = me:map()
        if map == nil then
            return true
        end

        local x, y = me:position()
        if #args == 0 then
            local id, obj, blocked = map:tile(x, y)
            if id == nil then
                return true
            end

            if blocked then
                blocked = 'true'
            else
                blocked = 'false'
            end
            me:message(string.format('타일 : %d\n오브젝트 : %d\n블록 : %s', id, obj, blocked), MESSAGE_TYPE_POPUP)
        else
            local value = table.unpack(args)
            map:tile(x, y, value)
        end
        return true
    end,

    ['서버저장'] = 
    function (me, args)
        save()
        return true
    end,

    ['랜덤이동'] = 
    function (me, args)
        local maps = maps()
        local map = maps[math.random(0, #maps-1)]
        local x = math.random(0, map:width())
        local y = math.random(0, map:height())
        me:map(map, x, y)
        return true
    end,

    ['엔피씨생성'] = 
    function (me, args)
        local name, map, x, y = table.unpack(args)
        if map == nil then
            map = me:map()
        end

        if x == nil or y == nil then
            x, y = me:position()
        end
        mknpc(name, map, x, y)
        return true
    end,

    ['내구도'] = 
    function (me, args)
        local percent = table.unpack(args)
        percent = math.max(0, math.min(100, tonumber(percent)))
        for parts, equipment in pairs(me:equipments()) do
            local model = equipment:model()
            equipment:durability(model:durability() * (percent / 100.0))
        end

        for _, item in pairs(me:items()) do
            local model = item:model()
            if model:attr(ITEM_ATTRIBUTE_EQUIPMENT) then
                item:durability(model:durability() * (percent / 100.0))
            end
        end
        return true
    end,

    ['sleep'] = 
    function (me, args)
        local time = table.unpack(args)
        sleep(time)
        me:message('done')
        return true
    end,

    ['광고'] = 
    function (me, args)
        local width, height, url, time = table.unpack(args)
        me:ad(width, height, url, time)
        return true
    end,

    ['웹'] = 
    function (me, args)
        local type, url, message = table.unpack(args)
        me:web(type, url)
        return true
    end,

    ['메일쓰기'] = 
    function (me, args)
        local to, title, contents = table.unpack(args)
        me:send_mail(to, title, contents)
        return true
    end,

    ['쿨타임초기화'] = 
    function (me, args)
        for slot, spell in pairs(me:spells()) do
            me:delay(spell, 0)
        end
        return true
    end,

    ['금전'] = 
    function (me, args)
        local money = table.unpack(args)
        me:money(me:money() + tonumber(money))
        return true
    end,

    ['성전환'] = 
    function (me, args)
        if me:sex() == SEX_MAN then
            me:sex(SEX_WOMAN)
        else
            me:sex(SEX_MAN)
        end
        return true
    end
}