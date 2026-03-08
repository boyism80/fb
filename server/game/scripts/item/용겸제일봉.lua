-- 용겸제일봉: on attack, chance to apply 용의제일노 (curse) on front target. Trash: 15% prob, 6s, effect 53, sound 40.
function ON_ATTACK_276(me, item)
    math.randomseed(seed())
    if math.random() > 0.15 then
        return false
    end
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        return
    end
    front:sound(40)
    front:effect(53)
    front:buff('용의제일노', 6, me)
end

-- 용겸제일봉 사용 스크립트
function ON_ACTIVE_276(me, item)

end

-- 용겸제일봉 해제 스크립트
function ON_INACTIVE_276(me, item)

end