{
    image 0, 7, 3;
    set @sd, mysd();
    set @id, myid();

    switch(get_job(@sd))
    {
    case 1:
        set @menu$, "현철중검 업그레이드";
        break;
    case 2:
        set @menu$, "야월도 업그레이드";
        break;
    case 3:
        set @menu$, "영혼마령봉 업그레이드";
        break;
    case 4:
        set @menu$, "해골죽장 업그레이드";
        break;
    }

    if(get_level(@sd) <= 50) { set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "물건 고치기", "모두 고치기", "무기 별칭 명명", "아이템 파괴", "뢰진도 만들기", "연호박보석만들기"); }
    else { set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "물건 고치기", "모두 고치기", "무기 별칭 명명", "아이템 파괴", "뢰진도만들기", "연호박보석만들기", @menu$); }

    if(@ret == 1)
    {
        set @ret, list1("무엇을 사시게요?", "기타");
        if(@ret == 1) { callshop 0, 22; }
    }
    else if(@ret == 2) { callshop 1, 23; }
    else if(@ret == 3) { callrepair(); }
    else if(@ret == 4) { callrepairall(); }
    else if(@ret == 5) { callnaming(); }
    else if(@ret == 6) { callfunc "DESTROY_ITEM"; }
    else if(@ret == 7)
    {
        if(countitem(name2itemid("벼락나뭇가지")) == 0)
        {
            set @pos, dialog(0, 0, 0, "뢰진도를 만들기 위한 재료가 없는데?");
            end;
        }
        else
        {
COS001:
            set @pos, dialog(0, 0, 1, "오옷, 이것은 벼락을 맞은 나뭇가지가 아닌가?\\n\\n이리 줘보게, 내 한번 깎아서 무기를 만들어볼테니..");
            if(@pos == 0) end;
COS002:
            set @pos, dialog(0, 1, 1, "싫다면 지금이라도 그만두게...");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS001;
COS003:
            set @pos, dialog(0, 1, 1, "...\\n\\n......\\n\\n.........");
            if(@pos == 0) end;
            else if(@pos == -1) goto COS002;
COS004:
            delitem @sd, name2itemid("벼락나뭇가지"), 1;
            message @sd, 3, "벼락나뭇가지 주었습니다.";
            if(rand(1, 10) <= 9)
            {
                
                additem @sd, name2itemid("뢰진도"), 1;
                set @pos, dialog(0, 0, 1, "아주 훌륭한 무기가 완성되었다네! 한번 휘둘러 보게나!");
                end;
            }
            else
            {
                additem @sd, name2itemid("천둥부적"), 1;
                set @pos, dialog(0, 0, 1, "아쉽게 실패했군. 하지만 쓸만한 보조류를 만들었으니 요긴하게 사용하게나.");
                end;
            }
        }
    }
    else if(@ret == 8)
    {
        set @pos, list2(@list, 0, 1, "어떤 색깔의 호박보석을 만드시겠어요?", "연갈호박보석입니다.", "연녹호박보석입니다.", "연자호박보석입니다.", "연청호박보석입니다.");
        if(@pos == 0) end;

        setarray @item$[1], "연갈호박", "연녹호박", "연자호박", "연청호박";

        if(countitem(name2itemid(@item$[@list] + "결정")) < 10)
        {
            set @pos, dialog(0, 0, 0, @item$[@list] + "결정이 부족합니다.");
        }
        else
        {
            delitem @sd, name2itemid(@item$[@list] + "결정"), 10;
            message @sd, 3, @item$[@list] + "결정 주었습니다";
            additem @sd, name2itemid(@item$[@list] + "보석"), 1;
            set @pos, dialog(0, 0, 1, @item$[@list] + "보석 만들어드렸습니다.");
        }
        deletearray @item$[1], getarraysize(@item$);
    }
    else if(@ret == 9)
    {
        callfunc "WEAPON_UPGRADE";
    }
}