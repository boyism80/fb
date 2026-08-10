-- map: 길림성 (유두절 폭포 목욕)
-- Warp tiles: (183-185,5-6), (176-178,5-6) -> on_yudu_bath
local festival = require('lib.festival')

local ACHIEVEMENT_BATH = 560

return {
    on_yudu_bath = function(me)
        if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end
        if not festival.is('유두') then
            return
        end
        if me:isbuff('반짝반짝') then
            return
        end

        me:buff('반짝반짝', 185)
        me:message('몸을 씻었습니다.', MESSAGE_TYPE.NOTIFY)
        me:push_achievement(ACHIEVEMENT_BATH, '길림폭포에서 목욕을 하다.', 7, 1)
    end,
}
