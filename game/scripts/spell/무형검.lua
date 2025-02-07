function on_cast(me, spell)
    local function preprocess(me, you)
        you:buff('무형술', 2, me)
    end

    local effect = 92
    local sound = 103
    local hp = me:hp() // 2
    local mp = 180
    local damage = me:hp() + math.min(20000, me:mp())

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '무형검', sound, effect, preprocess) then
        return
    end
end