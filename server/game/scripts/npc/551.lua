-- npc: 암흑귀인
local quest = require('lib.quest')
local PARENT_MAP_GWIMUN = "둔귀촌"

return {
    ON_CLICK = function(me, npc)
        local cur = me:map()
        if cur then
            local root = cur:model():root()
            if root == nil or root:name() ~= PARENT_MAP_GWIMUN then
                me:dialog(npc, "핵 사용자로 운영자에게 보고 됩니다.", { prev = false, next = false })
                return
            end
        end
        local q = me:quest(quest.QUEST_GMD_SAVED)
        local saved_map = (q and q:param() and q:param() ~= "") and q:param() or "귀문암동대기실"
        local sel, btn = me:list(npc, "[" .. saved_map .. "]에 이동하려면 [귀문흑색부적]이 한개 필요하다네. 어떻게 하겠는가?", {
            "예. 보내주세요.",
            "아니오. 괜찮습니다.",
        }, { prev = false })
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            if not me:has_items("귀문흑색부적", 1) then
                me:dialog(npc, "자네는 귀문흑색부적을 가지고 있지 않은 것 같군...", { prev = false, next = false })
                return
            end
            if not me:rmitem("귀문흑색부적", 1, ITEM_DELETE_TYPE.GIVE) then
                return
            end
            local map = name2map(saved_map)
            if map == nil then
                me:mkitem("귀문흑색부적", 1)
                me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
                return
            end
            local x = math.random(10, 15)
            local y = math.random(12, 20)
            me:map(map, x, y)
            q = me:start_quest(quest.QUEST_GMD_SAVED)
            if q then
                q:param("귀문암동대기실")
            end
        else
            me:dialog(npc, "그래. 잘 생각해 보게나...", { prev = false, next = false })
        end
    end
}
