-- mob: 백현귀랑

return {
    on_mob_attack = function(me, you)
        if math.random(5) ~= 1 then
            return
        end
        me:chat('쮜이이이이이이익')
    end,
}
