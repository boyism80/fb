{
    set @sd, mysd();
    set @id, myid();

    if($tongtong == 0)
    {
        set @pos, dialog(0, 0, 1, "안녕하신가, 나는 무기 수집가라네. 자네 대륙 사람 맞지?");
        if(@pos == 0) end;

        set @pos, list2(@list, 0, 0, "대륙에 망치라는 도구가 있다던데 쓸모가 많다더군. 혹시 하나 구해다 줄 수 있나?", "네, 구해드리겠습니다.", "글쎄요...");
        if(@pos == 0) end;
        else if(@list != 1) end;

        set $tongtong, 1;
        set @pos, dialog(0, 0, 1, "이 일기같은건 잘 보관하고 있을테니, 조심해서 다녀오시게.");
        end;
    }
    if($tongtong == 1)
    {
        if(countitem(name2itemid("망치")) == 0)
        {
            set @pos, dialog(0, 0, 0, "아직 망치라는 것을 구하지 못한것 같군?");
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "과연! 묵직하고 단단한 것이 무기로도 손색이 없겠어!");
            if(@pos == 0) end;

            set $tongtong, 2;
            delitem @sd, name2itemid("망치"), 1;
            message @sd, 3, "망치 주었습니다";
            additem @sd, name2itemid("선장의일기3"), 1;
            addlegend @sd, 32, 7, 0, 1, "진백랑의 부탁을 들어주자. [4/6]";
            set @pos, dialog(0, 0, 1, "이건 약속했던 누군가의 일기일세! 잘 가시게!");
            end;
        }
    }
    else
    {
        set @pos, dialog(0, 0, 1, "흠..이 단단하고 묵직한 망치. 정말 쓸 곳이 많구만.");
        end;
    }
}