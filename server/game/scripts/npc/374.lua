-- npc: 소림사둔갑술
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local btn = me:dialog(npc, "나무아미타불.. 소림사에 입문하신 수행자이시군요. 백열장의 수련은 잘 되고 계신지?", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local sel, list_btn = me:list(npc, "제가 도와드릴 일이라도 있겠습니까?", {
            "둔갑술이 무엇인가요?",
            "둔갑술을 배우고 싶습니다.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end

        if sel == 1 then
            btn = me:dialog(npc, "이전에 고서적을 보던중에, 일본의 닌자라는 집단이 사용하는 '둔갑술'에 대해서 본 적이 있습니다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "이것을 소림사의 무술과 결합시켜, 특별한 수행없이 특정한 모습으로 변신할 수 있는 둔갑술을 사용할 수 있게 되었지요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그러나 수행이 없는 대신에 '소림오행진'이라는 특별한 도구가 필요합니다. 이것으로 상대방의 모습을 기억할 수 있게 되지요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            me:dialog(npc, "이 '소림오행진'은 변하고 싶은 것이 어떤 모습이냐에 따라 조금씩 재료가 달라집니다. 그리고 굉장이 불안정하기 때문에 쉽게 깨지곤 하지요.", { prev = false, next = false })
            return
        end

        if sel == 2 then
            btn = me:dialog(npc, "간단한 둔갑술은 제가 가르쳐드릴수 있습니다만, 특별한 모습으로 변하고 싶으시다면 소림오행진을 착용하고 직접 가서 모습을 확인하셔야합니다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel2, list_btn2 = me:list(npc, "그럼 어떤 둔갑술을 배우고 싶으신가요?", {
                "동물들로 변하고 싶습니다.",
                "천구로 변하고 싶습니다.",
                "후로 변하고 싶습니다.",
                "강시로 변하고 싶습니다.",
                "다른 모습으로 변하고 싶습니다.",
            }, { prev = false })
            if list_btn2 == DIALOG_RESULT.QUIT or sel2 == nil then
                return
            end

            if sel2 == 1 then
                if me:spell("소림경수둔갑술") then
                    me:dialog(npc, "이미 배운 마법이군요.", { prev = false, next = false })
                    return
                end
                btn = me:dialog(npc, "동물이라면.. 고양이, 코뿔소, 악어, 살쾡이 정도가 있겠군요..", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "둔갑술을 배우는데는 몇가지 도구가 필요합니다. 제가 소림오행진을 만들 재료와 이전에 모아둔 동물들의 모습을 이용하여 둔갑술을 완성해야 하기 때문이지요.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "동물로 변하는 둔갑술에는 구기자5개, 동동주 3병, 금전 200전이 필요합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if not me:has_items("구기자", 5) then
                    me:dialog(npc, "구기자를 구해오세요. 현도성의 상점에서 팔고 있을겁니다.", { prev = false, next = false })
                    return
                end
                if not me:has_items("동동주", 3) then
                    me:dialog(npc, "동동주를 구해오세요. 방장실 앞에 있는 술장수가 팔고 있을 겁니다.", { prev = false, next = true })
                    return
                end
                if me:money() < 200 then
                    me:dialog(npc, "금전이 부족하군요.. 금전을 통해서 소림사의 유지에 쓰입니다.", { prev = false, next = true })
                    return
                end
                if not me:rmitem("구기자", 5, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("동동주", 3, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "재료를 건네지 못했습니다.", { prev = false, next = false })
                    return
                end
                me:money(me:money() - 200)
                me:mkspell("소림경수둔갑술")
            elseif sel2 == 2 then
                if me:spell("소림천구둔갑술") then
                    me:dialog(npc, "이미 배운 마법이군요.", { prev = false, next = false })
                    return
                end
                btn = me:dialog(npc, "천구로 변하고 싶으십니까. 천구는 사실 익수입니다만, 천지인패의 영향인지 최근 사람을 공격한다고 합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "천구로 변하는 둔갑술에는 구기자 10개, 백세주 5병, 금전 1000전이 필요합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if not me:has_items("구기자", 10) then
                    me:dialog(npc, "구기자를 구해오세요. 현도성의 상점에서 팔고 있을겁니다.", { prev = false, next = false })
                    return
                end
                if not me:has_items("백세주", 5) then
                    me:dialog(npc, "백세주를 구해오세요. 방장실 앞에 있는 술장수가 팔고 있을 겁니다.", { prev = false, next = true })
                    return
                end
                if me:money() < 1000 then
                    me:dialog(npc, "금전이 부족하군요.. 금전을 통해서 소림사의 유지에 쓰입니다.", { prev = false, next = true })
                    return
                end
                if not me:rmitem("구기자", 10, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("백세주", 5, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "재료를 건네지 못했습니다.", { prev = false, next = false })
                    return
                end
                me:money(me:money() - 1000)
                me:mkspell("소림천구둔갑술")
            elseif sel2 == 3 then
                if me:spell("소림후둔갑술") then
                    me:dialog(npc, "이미 배운 마법이군요.", { prev = false, next = false })
                    return
                end
                btn = me:dialog(npc, "후로 변하고 싶으십니까. 후는 강시가 동물로 변한 것이라고 하는데, 최근에 굉장히 흉폭해졌다고 하더군요.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "후로 변하는 둔갑술에는 구기자 20개, 백세주 10병, 금전 5000전이 필요합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if not me:has_items("구기자", 20) then
                    me:dialog(npc, "구기자를 구해오세요. 현도성의 상점에서 팔고 있을겁니다.", { prev = false, next = false })
                    return
                end
                if not me:has_items("백세주", 10) then
                    me:dialog(npc, "백세주를 구해오세요. 방장실 앞에 있는 술장수가 팔고 있을 겁니다.", { prev = false, next = true })
                    return
                end
                if me:money() < 5000 then
                    me:dialog(npc, "금전이 부족하군요.. 금전을 통해서 소림사의 유지에 쓰입니다.", { prev = false, next = true })
                    return
                end
                if not me:rmitem("구기자", 20, ITEM_DELETE_TYPE.GIVE) or not me:rmitem("백세주", 10, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "재료를 건네지 못했습니다.", { prev = false, next = false })
                    return
                end
                me:money(me:money() - 5000)
                me:mkspell("소림후둔갑술")
            elseif sel2 == 4 then
                btn = me:dialog(npc, "강시는 제가 수행을 하던때에는 나타나지 않아서 모습을 알지 못한답니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "강시의 모습으로 변하고 싶으시다면, 소림오행진을 장갑이나 팔찌대신 장비하시고 강시를 잡으면, 소림오행진이 깨어지면서 강시의 모습을 담음 소림오행진으로 바뀌게 됩니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "조심하실 것은, 소림오행진은 매우 불안정한 물건이기 때문에, 쉽게 깨어집니다. 죽었을 경우는 물론이고, 내구력도 매우 낮아, 공격을 받다보면 쉽게 깨어지니 절대 주의하시기 바랍니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "그럼 소림오행진을 받으시겠으면 다음을 눌러주시길 바랍니다. 소림오행진을 제작하는데에는 5만전이 들기 때문에 5만전을 시주하셔야 합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                local code = me:exchange(
                    { ['money'] = 50000 },
                    { ['item'] = { ["소림오행진"] = 1 } }
                )
                if code == enum.EXCHANGE_RESULT.LACK_COST then
                    me:dialog(npc, "금전이 부족하군요.. 금전을 통해서 소림사의 유지에 쓰입니다.", { prev = false, next = true })
                    return
                end
                if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                    me:dialog(npc, "소지품이 가득 차서 소림오행진을 받을 수 없습니다.", { prev = false, next = true })
                    return
                end
            elseif sel2 == 5 then
                btn = me:dialog(npc, "다른 모습으로 변하시고 싶으시다구요.. 저는 잘 모르겠습니다만, 형태에 따라서 어떤 동물이나 사람으로 변할 수 있다고 합니다. 하지만 안되는 경우가 더 많을 것이니, 수련을 통해 하나하나 찾는 수 밖에 없습니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "북방대초원과 같은 곳에 잘못 들어갔다가 소림오행진만 깨져버릴 위험이 굉장이 큽니다. 그리고, 원하는 모습으로 변할 수 있을지 없을지도 알수 없습니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "그래도 도전해 보고 싶으시다면, 제가 특별히 제작한 진소림오행진을 가지고 가보십시오. 이것을 제작하는 데에는 굉장한 긴 시일과 많은 재료가 들어가기 때문에, 50만전을 시주하셔야 합니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, "진소림오행진을 받고 싶으시다면 다음을 눌러주시길 바라며, 비용은 50만전이니 신중하게 선택하여 주시길 바랍니다.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                local code = me:exchange(
                    { ['money'] = 500000 },
                    { ['item'] = { ["진소림오행진"] = 1 } }
                )
                if code == enum.EXCHANGE_RESULT.LACK_COST then
                    me:dialog(npc, "금전이 부족하군요.. 금전을 통해서 소림사의 유지에 쓰입니다.", { prev = false, next = true })
                    return
                end
                if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                    me:dialog(npc, "소지품이 가득 차서 진소림오행진을 받을 수 없습니다.", { prev = false, next = true })
                    return
                end
            else
                me:dialog(npc, "준비중입니다.", { prev = false, next = false })
                return
            end

            btn = me:dialog(npc, "여기 있습니다. 잘 쓰시길 바라며, 수행이 필요 없을지라도 꾸준한 수행은 자신의 진의를 깨달을 수 있는 길입니다. 나무아미타불..", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
