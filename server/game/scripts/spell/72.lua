-- spell: 견우직녀축복
-- Short blessing granted when 견우/직녀 reunite on 오작교.
-- On expire: change look and warp to 견우직녀의집.

return {
    on_cast = function(me, you, sp)
        local target = you or me
        if target == nil then
            return false
        end
        target:buff(sp, 7, me)
        return true
    end,

    on_buff = function(me, sp)
    end,

    on_unbuff = function(me, sp)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        if me:gender() == GENDER.MALE then
            me:look(19)
        else
            me:look(89)
        end

        local house = id2map(10960) or name2map('견우직녀의집')
        if house ~= nil then
            me:map(house, math.random(7, 12), math.random(6, 15))
        end
    end,
}
