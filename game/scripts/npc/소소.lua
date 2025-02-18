function on_interact(me, npc)
::ROOT::
    local index, button = npc:list(me, '안녕하세요. 무엇을 도와드릴까요?', {'머리염색'}, false)
    if index == nil then
        return
    end

    if index == 0 then
        local hair_color = 0
        while true do
            local preset = { ['hair_color'] = hair_color }
            index, button = npc:list(me, '다음에 또 수술을 해 줄 수는 있지만, 수술을 계속한다고 결코 좋아지지는 않는다. 이 모습을 선택하겠느냐?', {'선택', '이전 모습으로', '다음 모습으로', '수술 포기'}, false, preset)
            if index == nil then
                return
            end

            if index == 0 then
                if npc:dialog(me, '그럼 네 얼굴을 그 모습을 고쳐주도록 하지.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                me:color(hair_color)
                if npc:dialog(me, '새 얼굴이 마음에 드는가? 맘에 안들어도 다시 해 줄 수는 없네.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end

                goto ROOT
            elseif index == 1 then
                hair_color = (hair_color - 1 + 32) % 32
            elseif index == 2 then
                hair_color = (hair_color + 1) % 32
            else
                return
            end
        end
    end
end