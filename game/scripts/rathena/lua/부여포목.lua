{
    set @sd, mysd();
    image 0, 6, 3;
COS001:
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "끈옷판매", "산타클로스옷판매", "호박결정만들기");
    if(@ret == 1)
    {
        set @ret, list1("무엇을 사시게요?", "전사용 갑주류", "전사용 비늘갑주류", "도적용 도복류", "도적용 갑옷류", "주술사용 치마류", "주술사용 도포류", "도사용 장삼류", "남자용 투구류");
        if(@ret == 1) { callshop 0, 2; }
        else if(@ret == 2) { callshop 0, 3; }
        else if(@ret == 3) { callshop 0, 4; }
        else if(@ret == 4) { callshop 0, 5; }
        else if(@ret == 5) { callshop 0, 6; }
        else if(@ret == 6) { callshop 0, 7; }
        else if(@ret == 7) { callshop 0, 8; }
        else if(@ret == 8) { callshop 0, 9; }
    }
    else if(@ret == 2) { callshop 1, 10; }
    else if(@ret == 3)
    {
        set @pos, dialog(0, 0, 1, "특별 이벤트 기간에만 가능합\\n니다.");
    }
    else if(@ret == 4)
    {
        set @pos, dialog(0, 0, 1, "특별 이벤트 기간에만 가능합\\n니다.");
    }
    else if(@ret == 5)
    {
        set @pos, list2(@list, 0, 1, "어떤 색깔의 호박결정을 만드시겠어요?", "연갈호박결정입니다.", "연녹호박결정입니다.", "연자호박결정입니다.", "연청호박결정입니다.");
        if(@pos == 0) end;

        setarray @item$[1], "연갈호박", "연녹호박", "연자호박", "연청호박";

        if(countitem(name2itemid(@item$[@list] + "단추")) < 15)
        {
            set @pos, dialog(0, 0, 0, @item$[@list] + "단추가 부족합니다.");
        }
        else
        {
            delitem @sd, name2itemid(@item$[@list] + "단추"), 15;
            message @sd, 3, @item$[@list] + "단추 주었습니다";
            additem @sd, name2itemid(@item$[@list] + "결정"), 1;
            set @pos, dialog(0, 0, 1, @item$[@list] + "결정 만들어드렸습니다.");
        }
        deletearray @item$[1], getarraysize(@item$);
    }
}