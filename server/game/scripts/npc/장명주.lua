
local function find_butterfly_item(me)
    for n = 10, 0, -1 do
        local name = "채집통 [" .. n .. " 마리]"
        if me:has_items(name, 1) then
            return name
        end
    end
    return nil
end

function NPC_359(me, npc)
    local btn = me:dialog(npc, "안녕하세요? 저는 장명주입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    if property("sesi_rightnow") ~= 3 then
        return
    end

    btn = me:dialog(npc, "안녕하세요? 화창한 봄이죠?", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "안녕하세요?", {
        "삼짇날에 대해서 알려주세요.",
        "나비를 잡아왔어요.",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        btn = me:dialog(npc, "삼짇날은 음력 3월 3일이에요. 날씨가 포근하고 개나리와 진달래가 만발하는 시기에 있는 명절이랍니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "음력 9월 9일 중앙절에 강남갔던 제비가 돌아오는 날이라고도 해요. 요즘 세시마을에서 제비가 날아다니는거 보셨나요?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "삼짇날엔 진달래꽃으로 떡을 만들어서 화전을 해먹기도 하죠. 혹은 녹두에 반죽해서 국수를 만들어먹기도 해요. 이것을 화면(花麵)이라고 하죠.", false, false)
        return
    end

    if sel == 1 then
        local item_name = find_butterfly_item(me)
        if item_name then
            local num = tonumber(item_name:match("%[(%d+)%]"))
            local cost = { ['item'] = { [item_name] = 1 } }
            local reward, code
            if num == 10 then
                reward = { ['item'] = { ["맛있는화면"] = 1, ["보통화면"] = 2 } }
            elseif num >= 4 and num <= 9 then
                reward = { ['item'] = { ["보통화면"] = 2 } }
            else
                reward = { ['item'] = { ["보통화면"] = 1, ["맛없는화면"] = 1 } }
            end
            code = me:exchange(cost, reward)
            if code == EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "나비를 가져오시면 감사하겠어요.", false, false)
                return
            end
            if code == EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 화면을 받을 수 없어요. 자리 좀 비우고 다시 오세요.", false, false)
                return
            end
            if num == 10 then
                btn = me:dialog(npc, "정말 감사해요!! 저희가 만든 화면을 드릴께요 ^^", false, true)
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                me:dialog(npc, "아, 그런데 나비채랑 채집통을 준 선릉이 오빠에게도 화면을 줬으면 하는데..", false, false)
            else
                me:dialog(npc, "정말 감사해요!! 저희가 만든 화면을 드릴께요 ^^", false, false)
            end
            return
        end

        btn = me:dialog(npc, "아유.. 이런 화창한 봄날에 집안에만 쳐박혀 있어야 한다니.. 정말 속상하네..", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그지 언니? 이게 뭐하는거야.. 우웅.. 요즘은 나비도 날아다니는거 같던데. 놀러가지도 못하고..", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "아 " .. me:name() .. "님. 손님이 계신데 저희끼리 얘기했네요. 죄송합니다. 놀러가고 싶은데 못가서 속상해서 그런거에요. 화면을 만들어도 기운이 없어서 그런지 제대로 안만들어지구요..", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "앗. 그럼 " .. me:name() .. "님이 나비를 좀 잡아주시겠어요? 그럼 기운이 나서 맛있는 화면을 만들 수 있을것 같은데. 헤헤헤", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sub_sel, sub_btn = me:list(npc, "나비 좀 잡아주세요오오오!!", {
            "네, 잡아드릴께요.",
            "나비 잡을시간 없는데..",
        }, false)
        if sub_btn == DIALOG_RESULT.QUIT or sub_sel == nil then
            return
        end
        if sub_sel == 0 then
            btn = me:dialog(npc, "와아 감사합니다!! 나비채랑 채집통은 선릉이 오빠한테 있는데.. 창고 있는 섣달집에요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "나비는 달맞이 고개에 있을꺼에요. 지금쯤 꽃이 활짝 펴서 나비들도 날아다니고 있을껄요?", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "그럼 부탁드립니다!! 기다릴께요!", false, false)
        else
            btn = me:dialog(npc, "네.. 그러시군요.. 어쩔 수 없죠 뭐.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "그래도 화면은 드려야 되는데.. 저희가 만들어 둔거라도 드릴께요. 맛있게 드세요.", false, false)
            me:mkitem("맛없는화면", 1)
        end
    end
end
