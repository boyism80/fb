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

        if x ~= nil and y ~= nil then
            me:map(map, tonumber(x), tonumber(y))
        else
            me:map(map)
        end
        return true
    end,

    ['사운드'] =
    function (me, args)
        local sound = table.unpack(args)
        me:sound(tonumber(sound))
        return true
    end,

    ['액션'] = 
    function (me, args)
        local action = table.unpack(args)
        me:action(tonumber(action))
        return true
    end,

    ['날씨'] = 
    function (me, args)
        local value = table.unpack(args)
        weather(tonumber(value))
        return true
    end,

    ['밝기'] = 
    function (me, args)
        local value = table.unpack(args)
        bright(tonumber(value))
        return true
    end,

    ['타이머'] = 
    function (me, args)
        local time = table.unpack(args)
        timer(tonumber(time), true)
        return true
    end,

    ['타이틀'] = 
    function (me, args)
        local value = table.unpack(args)
        me:title(tonumber(value))
        return true
    end,

    ['이펙트'] =
    function (me, args)
        local value = table.unpack(args)
        me:effect(tonumber(value))
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
        me:level(tonumber(level))
        return true
    end,

    ['힘바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_str(tonumber(value))
        return true
    end,

    ['민첩바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_dex(tonumber(value))
        return true
    end,

    ['지력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_int(tonumber(value))
        return true
    end,

    ['체력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_hp(tonumber(value))
        me:hp(me:maxhp())
        if me:state() == STATE_GHOST then
            me:state(STATE_NORMAL)
        end
        return true
    end,

    ['마력바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:base_mp(tonumber(value))
        me:mp(me:maxmp())
        return true
    end,

    ['아이템생성'] = 
    function (me, args)
        local name, count = table.unpack(args)
        me:mkitem(name, tonumber(count))
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
        me:script("script.lua", "func", 1, "hello", "good")
        return true
    end,

    ['머리바꾸기'] = 
    function (me, args)
        local value = table.unpack(args)
        me:look(tonumber(value))
        return true
    end,

    ['머리염색'] = 
    function (me, args)
        local value = table.unpack(args)
        me:color(tonumber(value))
        return true
    end,

    ['갑옷염색'] = 
    function (me, args)
        local value = table.unpack(args)
        me:armor_color(tonumber(value))
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
            map:tile(x, y, tonumber(value))
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
        else
            map = name2map(name)
        end

        if x == nil or y == nil then
            x, y = me:position()
        else
            x = tonumber(x)
            y = tonumber(y)
        end
        mknpc(name, map, x, y)
        return true
    end,

    ['내구도'] = 
    function (me, args)
        local percent = table.unpack(args)
        percent = tonumber(percent)
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
        sleep(tonumber(time))
        me:message('done')
        return true
    end,

    ['광고'] = 
    function (me, args)
        local width, height, url, time = table.unpack(args)
        me:ad(tonumber(width), tonumber(height), url, tonumber(time))
        return true
    end,

    ['웹'] = 
    function (me, args)
        local type, url, message = table.unpack(args)
        me:web(tonumber(type), url)
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
            me:spell(spell):delay(0)
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
    end,

    ['서버종료'] = 
    function (me, args)
        local delay = table.unpack(args)
        if delay == nil then
            delay = 10
        else
            delay = tonumber(delay)
        end

        while delay > 0 do
            if delay > 600 then
                term = math.min(600, delay-600)
            elseif delay > 60 then
                term = math.min(60, delay-60)
            elseif delay > 10 then
                term = math.min(10, delay-10)
            else
                term = 1
            end

            times = delay
            hours = times // 3600
            times = times - (hours * 3600)

            mins = times // 60
            times = times - (mins * 60)

            secs = times % 60
            message = ''
            if hours > 0 then
                message = string.format('%d시간', hours)
            end

            if mins > 0 then
                message = string.format('%s %d분', message, mins)
            end

            if secs > 0 then
                message = string.format('%s %d초', message, secs)
            end

            broadcast(string.format('[공지] %s 후 서버가 종료됩니다.', message), MESSAGE_TYPE_NOTIFY, BROADCAST_TYPE_GLOBAL)
            sleep(term * 1000)
            delay = delay - term
        end
        shutdown()
        return true
    end
}