{
    set @sd, mysd();
    set @id, myid();
    set @list, list1("안녕하신가? 나에겐 어쩐일로 왔는가?", "시계 구입", "건전지 구입");
    if(@list == 1)
    {
COS001:
        set @pos, dialog(0, 0, 1, "자네도 내가 만든 시계를 구입하고 싶어서 찾아온 게로구만? 그렇다면 잘 찾아왔네.");
        if(@pos == 0) end;

        set @pos, dialog(0, 1, 1, "그렇다면 잘 찾아왔구만. 하지만 공짜로 줄 순 없지.. 1만전만 낸다면야 생각은 해 보겠다만..");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;

        set @list_2, list1("1만전을 내고 나에게 시계를 사겠나?", "네, 살래요.", "안살래요.");
        if(@list_2 == 1)
        {
            if(get_money(@sd) < 10000)
            {
                set @pos, dialog(0, 0, 0, "아니.. 돈도 없이 내 시계를 살 순 없지.");
                end;
            }
            else
            {
                additem @sd, name2itemid("장인의회중시계"), 1;
                set_money @sd, get_money(@sd)-10000;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "여기 있네. 요긴하게 잘 사용하게나.");
                end;
            }
        }
    }
    else if(@list == 2)
    {
        set @pos, dialog(0, 0, 1, "안녕하신가? 나는 매일 오후 6시마다 나의 시계에 사용할수 있는 건전지를 10개씩 매입해온다네.");
        if(@pos == 0) end;

        set @pos, list2(@ret, 0, 0, "건전지는 하나당 5000전이고, 현재 건전지는 " + #clock_time_item + "개 있다네. 구입할텐가?", "네, 건전지 주세요.", "아니요, 아직 시계의 수명이 넉넉해서요.");
        if(@pos == 1 && @ret == 1)
        {
            if(#clock_time_item != 0)
            {
                if(get_money(@sd) < 5000)
                {
                    set @pos, dialog(0, 0, 0, "돈이 부족하구만.. 나의 건전지는 하나당 5000전이라네.");
                    end;
                }
                else
                {
                    additem @sd, name2itemid("건전지"), 1;
                    set_money @sd, get_money(@sd)-5000;
                    updatestatus @sd, 2;
                    set #clock_time_item, #clock_time_item-1;
                    set @pos, dialog(0, 0, 1, "자네 생각보다 운이 좋은걸? 여기 건전기 가져가게나.");
                    end;
                }
            }
            else
            {
                set @pos, dialog(0, 0, 0, "지금은 다 떨어졌네.. 다음에 다시 오게나.");
                end;
            }
        }
    }
}