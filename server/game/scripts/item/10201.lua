-- item: 대장군의일기1

return {
    ON_ACTIVATED = function(me, item)
        local model = item:model()
        local button = nil
    ::ON_ACTIVATED_10201_001::
        button = me:dialog(model, '-궐기 전야\n이미 천왕은 통치자로서의 분별력을 잃은 지 오래, 사리사욕을 채우기 급급하여 백성들의 피를 빠는 폭군이 되고 말았다.\n다른 영지를 통치하는 영주들도 백성들이 마음놓고 살 수 있도록 영지를 잘 다스리기는 커녕, 오히려 서로 영지를 빼앗으려는 크고 작은 전쟁을 일으켜 천하의 백성들을 슬프게 하고 있다. 천하는 이미 전쟁으로 가득한, 이른바 전국시대에 접어들었다.', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    ::ON_ACTIVATED_10201_002::
        button = me:dialog(model, '이런 시대에 대장군인 내가 할 수 있는 일은 단 하나, 난세를 평정해 천하통일을 이룩하는 것 뿐.\n\n그렇게 하면 더 이상의 전쟁의 고통에 괴로워하는 이도 없을터', { prev = true, next = false })
        if button == DIALOG_RESULT.PREV then
            goto ON_ACTIVATED_10201_001
        end
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
