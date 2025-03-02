-- 이기어검 캐스팅
function ON_CAST_2018(me, spell)
    local function preprocess(me, you)
        you:buff('이기어검술', 2, me)
    end

    local effect = 68
    local sound = 91
    local hp = me:hp() // 2
    local mp = 180
    local damage = me:hp() + math.min(20000, me:mp())

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '이기어검', sound, effect, preprocess) then
        return
    end
end