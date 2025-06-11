-- 헬파이어 캐스팅
function ON_CAST_3043(me, you, spell)
    if me:is(OBJECT_TYPE_CHARACTER) then
        local weapon = me:weapon()
        local delay = 7
        if weapon ~= nil then
            local name = weapon:name()
            if name == '용랑제삼봉' then
                delay = delay - 1
            elseif name == '용랑제사봉' then
                delay = delay - 1
            elseif name == '용랑제오봉' then
                delay = delay - 2
            elseif name == '용랑제육봉' then
                delay = delay - 2
            elseif name == '용랑제칠봉' then
                delay = delay - 3
            elseif name == '용랑제팔봉' then
                delay = delay - 3
            elseif name == '용랑제구봉' then
                delay = delay - 4
            end
        end
        me:spell(spell):delay(delay)
    end

    local damage = math.floor(me:mp() * 1.5)
    local mp = me:mp()
    local sound = 73
    local effect = 8
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end