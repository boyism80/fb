-- 초혼비무 캐스팅
function ON_CAST_1018(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local hp = me:hp()*2 // 3
    local mp = 140
    local effect = 105
    local sound = 99
    local damage = me:hp() * 3
    local message = '초혼비무'
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    for i = 1, 3 do
        if not map:movable(me, i) then
            return failed_attack_spell(me)
        end
    end

    local nears = me:nears(0xFF & ~OBJECT_TYPE_ITEM)
    local x, y = me:position()
    local direction = me:direction()
    local front = front_obj(x, y, direction, 4, nears, OBJECT_TYPE_LIFE)
    if front == nil then
        return failed_attack_spell(me)
    end

    local hp_consume = (me:hp() * 2) // 3
    me:hp(math.max(1, me:hp() - hp_consume))

    if direction == DIRECTION_LEFT then
        x = x-1
    elseif direction == DIRECTION_RIGHT then
        x = x+1
    elseif direction == DIRECTION_TOP then
        y = y-1
    else
        y = y+1
    end 

    if front:is(OBJECT_TYPE_LIFE) then
        front:position(x, y)
    end

    attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect})
end