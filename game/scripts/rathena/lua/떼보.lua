{
    set @sd, mysd();
    image 0, 11, 3;
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "호박별만들기");
    if(@ret == 1)
    {
        set @ret, list1("무엇을 사시게요?", "전사용 갑주류", "전사용 비늘갑주류", "도적용 도복류", "도적용 갑옷류", "주술사용 치마류", "주술사용 도포류", "도사용 장삼류", "남자용 투구류");
        if(@ret == 1) { callshop 0, 11; }
        else if(@ret == 2) { callshop 0, 12; }
        else if(@ret == 3) { callshop 0, 13; }
        else if(@ret == 4) { callshop 0, 14; }
        else if(@ret == 5) { callshop 0, 15; }
        else if(@ret == 6) { callshop 0, 16; }
        else if(@ret == 7) { callshop 0, 17; }
        else if(@ret == 8) { callshop 0, 18; }
    }
    else if(@ret == 2) { callshop 1, 19; }
    else if(@ret == 3)
    {
        set @pos, list2(@list, 0, 1, "어떤 색깔의 호박별을 만드시겠어요?", "황호박별입니다.", "적호박별입니다.", "녹호박별입니다.", "청호박별입니다.", "회호박별입니다.", "갈호박별입니다.", "자호박별입니다.");
        if(@pos == 0) end;

        setarray @item$[1], "황호박", "적호박", "녹호박", "청호박", "회호박", "갈호박", "자호박";

        if(countitem(name2itemid(@item$[@list] + "보석")) < 5)
        {
            set @pos, dialog(0, 0, 0, @item$[@list] + "보석이(가) 부족합니다.");
        }
        else
        {
            delitem @sd, name2itemid(@item$[@list] + "보석"), 5;
            message @sd, 3, @item$[@list] + "보석 주었습니다";
            additem @sd, name2itemid(@item$[@list] + "별"), 1;
            set @pos, dialog(0, 0, 1, @item$[@list] + "별 만들어드렸습니다.");
        }
        deletearray @item$[1], getarraysize(@item$);
    }
}