{
    set @sd, mysd();
    set @id, myid();
COS001:
    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "호박방어구만들기", "연호박방어구만들기", "황금호박방어구만들기");
    if(@ret == 1) { callshop 0, 44; }
    else if(@ret == 2) { callshop 1, 45; }
    else if(@ret == 3)
    {
        set @pos, list2(@list, 0, 1, "그럼.. 어떤 색깔의 호박결정을 가지고 왔는가?", "황호박결정입니다.", "적호박결정입니다.", "녹호박결정입니다.", "청호박결정입니다.", "회호박결정입니다.", "갈호박결정입니다.", "자호박결정입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_name$[1], "장갑", "팔찌", "반지", "선류";
            setarray @clothes_color$[1], "황호박", "적호박", "녹호박", "갈호박", "청호박", "회호박", "자호박";

            set @pos, list2(@make, 0, 0, "그래.. " + @clothes$[@list] + "결정으로 어떤 아이템을 만들텐가?", @clothes_color$[@list] + "장갑입니다.", @clothes_color$[@list] + "팔찌입니다.", @clothes_color$[@list] + "반지입니다.", @clothes_color$[@list] + "선류입니다.");
            if(@pos == 1 && @make > 0)
            {
                if(countitem(name2itemid(@clothes_color$[@list] + "결정")) == 0)
                {
                    set @pos, dialog(0, 0, 0, @clothes_color$[@list] + "결정이 없는것 같군.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@list] + "결정"), 1;
                    message @sd, 3, @clothes_color$[@list] + "결정 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@list] + @clothes_name$[@make]), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@list] + "결정으로 " + @clothes_color$[@list] + "" + @clothes_name$[@make] + " 만들어주었네.");
                }
            }
        }
        deletearray @clothes_name$[1], getarraysize(@clothes_name$);
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
    else if(@ret == 4)
    {
        set @pos, list2(@list, 0, 1, "그럼.. 어떤 색깔의 호박결정을 가지고 왔는가?", "연녹호박결정입니다.", "연청호박결정입니다.", "연갈호박결정입니다.", "연자호박결정입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_name$[1], "장갑", "팔찌", "반지", "선류";
            setarray @clothes_color$[1], "연녹호박", "연청호박", "연갈호박", "연자호박";

            set @pos, list2(@make, 0, 0, "그래.. " + @clothes$[@list] + "결정으로 어떤 아이템을 만들텐가?", @clothes_color$[@list] + "장갑입니다.", @clothes_color$[@list] + "팔찌입니다.", @clothes_color$[@list] + "반지입니다.", @clothes_color$[@list] + "선류입니다.");
            if(@pos == 1 && @make > 0)
            {
                if(countitem(name2itemid(@clothes_color$[@list] + "결정")) == 0)
                {
                    set @pos, dialog(0, 0, 0, @clothes_color$[@list] + "결정이 없는것 같군.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@list] + "결정"), 1;
                    message @sd, 3, @clothes_color$[@list] + "결정 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@list] + @clothes_name$[@make]), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@list] + "결정으로 " + @clothes_color$[@list] + "" + @clothes_name$[@make] + " 만들어주었네.");
                }
            }
        }
        deletearray @clothes_name$[1], getarraysize(@clothes_name$);
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
    else if(@ret == 5)
    {
        set @pos, list2(@list, 0, 1, "그럼.. 어떤 색깔의 호박결정을 가지고 왔는가?", "황금호박결정입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_name$[1], "장갑", "팔찌", "반지", "선류";
            setarray @clothes_color$[1], "황금호박";

            set @pos, list2(@make, 0, 0, "그래.. " + @clothes$[@list] + "결정으로 어떤 아이템을 만들텐가?", @clothes_color$[@list] + "장갑입니다.", @clothes_color$[@list] + "팔찌입니다.", @clothes_color$[@list] + "반지입니다.", @clothes_color$[@list] + "선류입니다.");
            if(@pos == 1 && @make > 0)
            {
                if(countitem(name2itemid(@clothes_color$[@list] + "결정")) == 0)
                {
                    set @pos, dialog(0, 0, 0, @clothes_color$[@list] + "결정이 없는것 같군.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@list] + "결정"), 1;
                    message @sd, 3, @clothes_color$[@list] + "결정 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@list] + @clothes_name$[@make]), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@list] + "결정으로 " + @clothes_color$[@list] + "" + @clothes_name$[@make] + " 만들어주었네.");
                }
            }
        }
        deletearray @clothes_name$[1], getarraysize(@clothes_name$);
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
}