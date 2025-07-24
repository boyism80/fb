-- 무형검 캐스팅
function ON_CAST_2020(me, spell)
    local function preprocess(me, you)
        you:buff('무형술', 2, me)
    end

    local effect = 92
    local sound = 103
    local hp = me:hp() // 2
    local mp = 180
    local damage = me:hp() + math.min(20000, me:mp())
    local message = '무형검'

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect, preprocess = preprocess}) then
        return
    end
end