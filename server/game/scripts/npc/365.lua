-- npc: 영문스님
local enum = require('lib.enum')
function NPC_365(me, npc)
    local btn = me:dialog(npc, "어서오오시게나... 자네는 무슨 일로 이곳까지 찾아오게 되었나?.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if property("sesi_rightnow") ~= 4 then
        return
    end

    local sel, list_btn = me:list(npc, "안녕하십니까.. 여긴 어쩐 일로 오셨나요?", {
        "석가탄신일이 어떤 날인가요?",
        "연등을 가지고 싶어요.",
        "연등에 들어가는 재료를 다 구해왔어요.",
    }, true)
    if list_btn == DIALOG_RESULT.QUIT or list_btn == DIALOG_RESULT.PREV then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        btn = me:dialog(npc, "음력 4월 8일은 부처님이 세상에 오신날입니다. '석가탄신일', '석탄일', '불탄일', '관불절', '초파일' 등의 많은 명칭이 있습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "우리나라의 모든 명절은 조상섬기기와 노은에 대한 공경이 큰 행사로 되어 있으며, 어린이를 위한 명절은 없었습니다. 그런데 4월 초파일은 어린이를 위한 잔칫날이 되었지요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "남자아이들에게는 피리, 오뚜기를, 여자아이들에게는 각시, 가마, 꽃등을 줬지요. 또 색등에 불을 켜서 높은 대를 세워 연등을 달고 색비단으로 기를 매달기도 했습니다.", false, false)
        return
    end

    if sel == 1 then
        btn = me:dialog(npc, "연등을 만들기 위해선 꿩의 꼬리깃털과 색비단이 필요합니다. 꿩은 지금쯤 세시밭에 나타날때가 된 것 같네요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "조심하셔야 하는것은 불가에선 살생을 금하고 있기 때문에 꿩을 죽이시면 안된다는 것입니다. 깃털만 얻어오십시오.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "색비단은 아마 촌장님의 집에서 주실겁니다.\n그럼...", false, false)
        return
    end

    if sel == 2 then
        btn = me:dialog(npc, "꿩의 깃털과 색비단을 가져오셨나요?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local code = me:exchange(
            { ['item'] = { ["색비단"] = 1, ["꿩의깃털"] = 1 } },
            { ['item'] = { ["연등"] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "연등을 만들기 위해서는 꿩의깃털, 색비단이 필요합니다.", false, false)
            return
        elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 연등을 줄 수 없습니다.", false, false)
            return
        end
        me:dialog(npc, "수고하셨습니다.. 그럼 어두운 밤길을 잘 비춰주는 연등이 되길... 나무아미타불...", false, false)
    end
end
