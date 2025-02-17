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
    ['경험치'] =
    function (me, args)
        local exp = table.unpack(args)
        exp = tonumber(exp)
        me:exp(me:exp() + exp)
        return true
    end,

    ['맵이동'] =
    function (me, args)
        local map, x, y = table.unpack(args)
        if name2map(map) == nil then
            return false
        end

        if x ~= nil and y ~= nil then
            me:map(map, x, y)
        else
            me:map(map)
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
        me:message('미구현')
        return true
    end,

    ['타이머'] = 
    function (me, args)
        local time = table.unpack(args)
        timer(time, true)
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
        me:message('미구현')
        return true
    end,

    ['마법지우기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['몬스터생성'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['직업바꾸기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['레벨바꾸기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['체력바꾸기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['마력바꾸기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['아이템생성'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['월드맵'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['스크립트'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['머리바꾸기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['머리염색'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['갑옷염색'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['서버종료'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['타일'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['서버저장'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['맵오브젝트'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['랜덤이동'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['엔피씨생성'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['내구도'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['동시성테스트'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['sleep'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['맵타일'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['광고'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['웹'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['메일쓰기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['메일읽기'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['메일삭제'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['쿨타임초기화'] = 
    function (me, args)
        me:message('미구현')
        return true
    end,

    ['금전'] = 
    function (me, args)
        me:message('미구현')
        return true
    end
}