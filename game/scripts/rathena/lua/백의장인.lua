{
    set @sd, mysd();
    set @id, myid();

    if(countitem(name2itemid("얼음")) == 0)
    {
        set @pos, dialog(0, 0, 0, "아이고.. 더워.. 본좌는 지금 더워서 아무것도 못하겠으니 그만 돌아가거라.");
        end;
    }
START:
    set @pos, list2(@list, 0, 0, "안녕하신가? 무엇을 도와줄까?", "상급 용무기를 각성시켜 주세요.", "호박무기만들기", "연호박무기만들기");
    if(@pos == 0) { end; }
    else if(@list == 1)
    {
COS001:
        set @pos, dialog(0, 0, 1, "상급 용무기를 각성시키러 왔다고? 그렇다면 잘 찾아왔네.");
        if(@pos == 0) { end; }
COS002:
        set @pos, dialog(0, 1, 1, "이제까지 나의 제자 천금장인에게 용무기를 맡기느라 수고가 많았네.");
        if(@pos == 0) end;
        else if(@pos == -1) { goto COS001; }
COS003:
        set @pos, dialog(0, 1, 1, "하지만 아무리 나라도 가끔 실수할 때가 있는법.");
        if(@pos == 0) end;
        else if(@pos == -1) { goto COS002; }
COS01:
        set @pos, list2(@list, 0, 0, "그래. 그래도 나에게 용무기 각성을 맡기겠나?", "네. 용무기를 각성해주세요.");
        if(@pos == 0) { end; }
        else if(@list != 1) { end; }
        else
        {
COS02:
            set @pos, list2(@lst, 0, 0, "좋아. 각오가 대단하군. 그럼 각성시킬 용무기를 고르게.", "용마", "용천", "용랑", "용겸");
            if(@pos == 0) { end; }
            else if(@lst == 1)
            {
                set @pos, list2(@choose, 0, 1, "어떤 무기를 각성할건가?", "용마제육검", "용마제칠검", "용마제팔검");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS02;

                setarray @success$[1], "용마제칠검", "용마제팔검", "용마제구검";
                setarray @failed$[1], "없어요", "용마제칠검(손상)", "용마제팔검(손상)";
                setarray @check$[1], "용마제육검", "용마제칠검", "용마제팔검";
            }
            else if(@lst == 2)
            {
                set @pos, list2(@choose, 0, 1, "어떤 무기를 각성할건가?", "용천제육검", "용천제칠검", "용천제팔검");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS02;

                setarray @success$[1], "용천제칠검", "용천제팔검", "용천제구검";
                setarray @failed$[1], "없어요", "용천제칠검(손상)", "용천제팔검(손상)";
                setarray @check$[1], "용천제육검", "용천제칠검", "용천제팔검";
            }
            else if(@lst == 3)
            {
                set @pos, list2(@choose, 0, 1, "어떤 무기를 각성할건가?", "용랑제육봉", "용랑제칠봉", "용랑제팔봉");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS02;

                setarray @success$[1], "용랑제칠봉", "용랑제팔봉", "용랑제구봉";
                setarray @failed$[1], "없어요", "용랑제칠봉(손상)", "용랑제팔봉(손상)";
                setarray @check$[1], "용랑제육봉", "용랑제칠봉", "용랑제팔봉";
            }
            else if(@lst == 4)
            {
                set @pos, list2(@choose, 0, 1, "어떤 무기를 각성할건가?", "용겸제육봉", "용겸제칠봉", "용겸제팔봉");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS02;

                setarray @success$[1], "용겸제칠봉", "용겸제팔봉", "용겸제구봉";
                setarray @failed$[1], "없어요", "용겸제칠봉(손상)", "용겸제팔봉(손상)";
                setarray @check$[1], "용겸제육봉", "용겸제칠봉", "용겸제팔봉";
            }
        }
        setarray @rand[1], 20, 8, 2;
        if(countitem(name2itemid("얼음")) == 0 || countitem(name2itemid("은나무가지")) == 0 || countitem(name2itemid(@check$[@choose])) == 0 || get_purehp(@sd) < 1000)
        {
            set @pos, dialog(0, 0, 0, "자네는 아직 용무기를 각성시킬 준비가 모자라군.");
        }
        else
        {
            if(rand(1, 100) <= @rand[@choose])
            {
                broadcast 5, get_name(@sd) + "님이 " + @check$[@choose] + " 강화에 성공하셨습니다.";
                delitem @sd, name2itemid("얼음"), 1;
                delitem @sd, name2itemid("은나무가지"), 1;
                delitem @sd, name2itemid(@check$[@choose]), 1;
                message @sd, 3, "얼음 주었습니다";
                message @sd, 3, "은나무가지 주었습니다";
                message @sd, 3, @check$[@choose] + " 주었습니다";
                additem @sd, name2itemid(@success$[@choose]), 1;
                set_purehp @sd, get_purehp(@sd)-1000;
                set_hp @sd, get_maxhp(@sd);
                set @pos, dialog(0, 0, 1, "축하하네. 하늘이 자네를 " + @success$[@choose] + "의 주인으로 인정하는군");
            }
            else
            {
                broadcast 5, get_name(@sd) + "님이 " + @check$[@choose] + " 강화에 실패하셨습니다.";
                delitem @sd, name2itemid("얼음"), 1;
                delitem @sd, name2itemid("은나무가지"), 1;
                delitem @sd, name2itemid(@check$[@choose]), 1;
                message @sd, 3, "얼음 주었습니다";
                message @sd, 3, "은나무가지 주었습니다";
                message @sd, 3, @check$[@choose] + " 주었습니다";
                if(name2itemid(@failed$[@choose]) > 0) { additem @sd, name2itemid(@failed$[@choose]), 1; }
                set_purehp @sd, get_purehp(@sd)-1000;
                set_hp @sd, get_maxhp(@sd);
                set @pos, dialog(0, 0, 0, "이런.. 용무기가 온도를 이기지 못하고 부숴저 버렸군..");
            }
        }
        deletearray @success$[1], getarraysize(@success$);
        deletearray @failed$[1], getarraysize(@failed$);
        deletearray @check$[1], getarraysize(@check$);
        deletearray @rand[1], getarraysize(@rand);
    }
    else if(@list == 2)
    {
        set @pos, list2(@make1, 0, 1, "그럼.. 어떤 색깔의 호박별을 가지고 왔는가?", "황호박별입니다.", "적호박별입니다.", "녹호박별입니다.", "청호박별입니다.", "회호박별입니다.", "갈호박별입니다.", "자호박별입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto START;
        if(@make1 > 0)
        {
            setarray @clothes_name$[1], "별검", "별도", "별봉", "별곤";
            setarray @clothes_color$[1], "황호박", "적호박", "녹호박", "갈호박", "청호박", "회호박", "자호박";

            set @pos, list2(@make2, 0, 0, "그래.. " + @clothes$[@make1] + "별로 어떤 아이템을 만들텐가?", @clothes_color$[@make1] + "별검입니다.", @clothes_color$[@make1] + "별도입니다.", @clothes_color$[@make1] + "별봉입니다.", @clothes_color$[@make1] + "별곤입니다.");
            if(@pos == 1 && @make2 > 0)
            {
                if(countitem(name2itemid(@clothes_color$[@make1] + "별")) == 0 || countitem(name2itemid("죽은지네")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "재료를 다시 한번 살펴보게. " + @clothes_color$[@make1] + "별과 죽은지네가 있어야 제작할 수 있다네.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@make1] + "별"), 1;
                    delitem @sd, name2itemid("죽은지네"), 1;
                    message @sd, 3, @clothes_color$[@make1] + "별 주었습니다";
                    message @sd, 3, "죽은지네 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@make1] + @clothes_name$[@make2]), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@make1] + "별로 " + @clothes_color$[@make1] + "" + @clothes_name$[@make2] + " 만들어주었네.");
                }
            }
        }
        deletearray @clothes_name$[1], getarraysize(@clothes_name$);
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
    else if(@list == 3)
    {
        set @pos, list2(@make1, 0, 1, "그럼.. 어떤 색깔의 호박별을 가지고 왔는가?", "연녹호박별입니다.", "연청호박별입니다.", "연갈호박별입니다.", "연자호박별입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto START;
        if(@make1 > 0)
        {
            setarray @clothes_name$[1], "별검", "별도", "별봉", "별곤";
            setarray @clothes_color$[1], "연녹호박", "연청호박", "연갈호박", "연자호박";

            set @pos, list2(@make2, 0, 0, "그래.. " + @clothes$[@make1] + "별로 어떤 아이템을 만들텐가?", @clothes_color$[@make1] + "별검입니다.", @clothes_color$[@make1] + "별도입니다.", @clothes_color$[@make1] + "별봉입니다.", @clothes_color$[@make1] + "별곤입니다.");
            if(@pos == 1 && @make2 > 0)
            {
                if(countitem(name2itemid(@clothes_color$[@make1] + "별")) == 0 || countitem(name2itemid("죽은지네")) == 0)
                {
                    set @pos, dialog(0, 0, 0, "재료를 다시 한번 살펴보게. " + @clothes_color$[@make1] + "별과 죽은지네가 있어야 제작할 수 있다네.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@make1] + "별"), 1;
                    delitem @sd, name2itemid("죽은지네"), 1;
                    message @sd, 3, @clothes_color$[@make1] + "별 주었습니다";
                    message @sd, 3, "죽은지네 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@make1] + @clothes_name$[@make2]), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@make1] + "별로 " + @clothes_color$[@make1] + "" + @clothes_name$[@make2] + " 만들어주었네.");
                }
            }
        }
        deletearray @clothes_name$[1], getarraysize(@clothes_name$);
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
}