-- mob: 후추귀신

local LINES = {
    "계피를 빼앗아가겠당~!!",
    "아무도 나를 방해 못해~!",
    "크크큭...!! 계피조아~!",
    "계피는 내가 가져가마~!",
    "고약은 못만들지...!!",
}

return {
    on_mob_attack = function(me, you)
        if me == nil then
            return
        end
        if math.random(1, 3) == 1 then
            me:chat(LINES[math.random(1, #LINES)])
        end
    end,

    on_mob_die = function(me)
        if me == nil then
            return
        end
        me:chat("안돼...! 내가 모은 계피가~!~!")

        local map = me:map()
        if map == nil then
            return
        end
        local mx, my = me:position()
        for dx = -5, 5, 2 do
            for dy = -5, 5, 2 do
                if not (dx == 0 and dy == 0) then
                    map:mkitem("계피", mx + dx, my + dy)
                end
            end
        end
    end,

    -- on_mob_action = function(me, you)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
