-- 폭류유성 캐스팅
local spell = require('lib.spell')

function ON_CAST_3046(me, sp)
    local damage = math.floor(me:hp()*0.75 + me:mp()*1.5)
    local hp = nil
    local mp = math.max(30, me:mp() // 2)
    local sound = 500
    local effect = 161

    if not spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 6, 5), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}}) then
    	return
    end
    
    me:hp(me:hp() // 2)
end