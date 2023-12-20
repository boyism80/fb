{
    set @sd, mysd();
    set @id, myid();
COS001:
    set @pos, list2(@list, 0, 0, "안녕하세요. 어떻게 오셨나요?", "얼음칼만들기", "초혼술방법", "호박투구만들기", "연호박투구만들기", "황금호박투구만들기");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        set @pos, list2(@ice, 0, 1, "그래..무슨일로 날 찾아 왔는가??", "얼음칼이 무엇인가요?", "얼음칼좀 만들어 주세요..", "그냥..인사차 왔어요...잘 계시죠? ^^");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        else if(@ice == 1)
        {
            set @pos, dialog(0, 0, 1, "얼음칼은 얼음으로 만들어진 칼로써 적을 마비시킬 수 있는 능력을 가지고 있지. 단점이 있다면 수리가 안된다고 해야하나? 허허..");
            end;
        }
        else if(@ice == 2)
        {
            if(countitem(name2itemid("얼음")) < 100)
            {
                set @pos, dialog(0, 0, 0, "얼음칼을 만들기 위해선 얼음 100개가 필요하네.");
                end;
            }
            else
            {
                set @pos, dialog(0, 0, 1, "오..모두 모아 왔구만.. 수고했네... 어디보자.. 전체적으로 얼음이 싱글탱글 하구먼.. 허허허.. 이보게..이 좋은 얼음이면..장식품으로 써도 좋아 보이는데..정말 얼음칼로 만들고 싶나?.. 흠.. 그래..할 수 없지... 만들어 주겠네...");
                if(@pos == 0) end;

                delitem @sd, name2itemid("얼음"), 100;
                message @sd, 3, "얼음 주었습니다";

                if(rand(1, 10) <= 8)
                {
                    additem @sd, name2itemid("얼음칼"), 1;
                    set @pos, dialog(0, 0, 0, "자..받게나..방금 만든 얼음칼일쎄...어떤가? 감사한가? 껄껄껄..그래..잘 가게..");
                    end;
                }
                else
                {
                    set @pos, dialog(0, 0, 0, "이런..제작 도중에 얼음이 모두 부숴저 버렸군.. 미안하네..");
                    end;
                }
            }
        }
        else if(@ice == 3)
        {
            set @pos, dialog(0, 0, 1, "허허..싱겁기는..");
            end;
        }
        else end;
    }
    else if(@list == 2)
    {
        if($water_ring_sucess != 1 || $shark_weapon == 0) end;

        if($shark_weapon == 1)
        {
            set @pos, list2(@weapon, 0, 0, " ", "혹시 상어장군...");
            if(@pos == 0) end;
            else if(@weapon != 1) end;

            set @pos, list2(@weapon_2, 0, 0, "아 그만 말해도 알겠네..한두명이 말해야 말이지.", "...");
            if(@pos == 0) end;
            else if(@weapon_2 != 1) end;

            set @pos, list2(@weapon_3, 0, 0, "상어장군 이야기 하는 거지?", "예 그렇습니다.");
            if(@pos == 0) end;
            else if(@weapon_3 != 1) end;

            set @pos, list2(@weapon_4, 0, 0, "물론 내가 알고 있기는 한데 말이야 그렇게 쉽게 알려 줄수는 없다네..", "...");
            if(@pos == 0) end;
            else if(@weapon_4 != 1) end;

            set @pos, list2(@weapon_5, 0, 0, "내가 매일같이 쉴 시간도 없이 여기 서 있느냐고 무척이나 몸이 허해져서 말이야.", "...");
            if(@pos == 0) end;
            else if(@weapon_5 != 1) end;

            set @pos, list2(@weapon_6, 0, 0, "보약이라도 한재 지어먹어야 할 것 같아.", "!!!");
            if(@pos == 0) end;
            else if(@weapon_6 != 1) end;

            set @pos, list2(@weapon_7, 0, 0, "그래서 말인데 자네가 용왕님께 지어드렸던 보약의 재료를 가지고 오면 내 방법을 알려주도록 하지.", "헉..별수없군요...", "차라리 모르고 말지..");
            if(@pos == 0) end;
            else if(@weapon_7 != 1) end;

            set $shark_weapon, 2;
            addlegend @sd, 24, 7, 0, 1, "한빙장의 부탁을 들어주자.";
COS002:
            set @pos, dialog(0, 0, 1, "보약의 재료는 나도 모르니까 자네가 직접 가서 물어보고 지어오라고.");
            if(@pos == 0) end;
COS003:
            set @pos, dialog(0, 1, 0, "아, 그리고 난 산삼도 한뿌리 먹었으면 좋겠으니까 가서 산삼도 한뿌리 가져오시게나.");
            if(@pos == -1) goto COS002;
            end;
        }
        if($shark_weapon == 2)
        {
            if(countitem(name2itemid("게집게")) == 0 || countitem(name2itemid("게등껍질")) == 0 || countitem(name2itemid("문어다리")) == 0 || countitem(name2itemid("해마꼬리")) == 0 || countitem(name2itemid("산삼")) == 0)
            {
                set @pos, dialog(0, 0, 0, "보약의 재료를 구하지 못한건가? 용왕님께 지어드렸던 보약의 재료와 산삼 한뿌리 가져오시게나.");
                end;
            }
            else
            {
                set $shark_weapon, 3;
                delitem @sd, name2itemid("게집게"), 1;
                delitem @sd, name2itemid("게등껍질"), 1;
                delitem @sd, name2itemid("문어다리"), 1;
                delitem @sd, name2itemid("해마꼬리"), 1;
                delitem @sd, name2itemid("산삼"), 1;
                message @sd, 3, "게집게 주었습니다";
                message @sd, 3, "게등껍질 주었습니다";
                message @sd, 3, "문어다리 주었습니다";
                message @sd, 3, "해마꼬리 주었습니다";
                message @sd, 3, "산삼 주었습니다";
                addlegend @sd, 24, 7, 0, 1, "한빙장의 부탁을 들어주었다.";
COS004:
                set @pos, dialog(0, 0, 1, "오. 재료를 다 모아왔군.\\n\\n그럼 잠시 기다리게나. 내 이걸 먼저 보약으로 만들어 먹고..");
                if(@pos == 0) end;
COS005:
                set @pos, dialog(0, 1, 1, ".\\n\\n..\\n\\n...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS004;
COS006:
                set @pos, dialog(0, 1, 1, "캬. 몸에 기운이 솟는구만. 솟아.\\n\\n그럼 받을대로 다 받았으니 슬슬 말해 주기로 하지.");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS005;
COS007:
                set @pos, dialog(0, 1, 1, "사실 그 제작방법은 상어장군만이 알고 있다고 하더군.\\n\\n하지만 상어장군이 죽은 이 마당에...");
                if(@pos == 0) end;
                else if(@pos == -1) goto COS006;
COS008:
                set @pos, dialog(0, 1, 1, "따라서 그를 저승에서 불러내서 물어봐야 한다네..\\n\\n그를 저승에서 불러내는 방법은 제단을 담당하고 있는 사람한테 가서 직접 물어보시게나.");
                if(@pos == -1) goto COS007;
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 1, "....");
            end;
        }
    }
    else if(@list == 3)
    {
        set @pos, list2(@make, 0, 1, "그럼.. 어떤 색깔의 호박보석을 가지고 왔는가?", "황호박보석입니다.", "적호박보석입니다.", "녹호박보석입니다.", "청호박보석입니다.", "회호박보석입니다.", "갈호박보석입니다.", "자호박보석입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_color$[1], "황호박", "적호박", "녹호박", "갈호박", "청호박", "회호박", "자호박";

            set @pos, list2(@make2, 0, 0, "그래.. " + @clothes$[@make] + "보석으로 어떤 아이템을 만들텐가?", @clothes_color$[@make] + "투구입니다.");
            if(@pos == 1 && @make2 == 1)
            {
                if(countitem(name2itemid(@clothes_color$[@make] + "보석")) == 0 || countitem(name2itemid("쇠가루")) < 2)
                {
                    set @pos, dialog(0, 0, 0, "자네 혹시 재료를 모르는 것인가? " + @clothes_color$[@make] + "보석과 쇠가루 2개를 가지고 와야 " + @clothes_color$[@make] + "투구를 만들어 줄 수 있네.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@make] + "보석"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    message @sd, 3, @clothes_color$[@make] + "보석 주었습니다";
                    message @sd, 3, "쇠가루 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@make] + "투구"), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@make] + "보석으로 " + @clothes_color$[@make] + "투구를 만들어주었네.");
                }
            }
        }
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
    else if(@list == 4)
    {
        set @pos, list2(@make, 0, 1, "그럼.. 어떤 색깔의 호박보석을 가지고 왔는가?", "연녹호박보석입니다", "연청호박보석입니다", "연갈호박보석입니다", "연자호박보석입니다");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_color$[1], "연녹호박", "연청호박", "연갈호박", "연자호박";

            set @pos, list2(@make2, 0, 0, "그래.. " + @clothes$[@make] + "보석으로 어떤 아이템을 만들텐가?", @clothes_color$[@make] + "투구입니다.");
            if(@pos == 1 && @make2 == 1)
            {
                if(countitem(name2itemid(@clothes_color$[@make] + "보석")) == 0 || countitem(name2itemid("쇠가루")) < 2)
                {
                    set @pos, dialog(0, 0, 0, "자네 혹시 재료를 모르는 것인가? " + @clothes_color$[@make] + "보석과 쇠가루 2개를 가지고 와야 " + @clothes_color$[@make] + "투구를 만들어 줄 수 있네.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@make] + "보석"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    message @sd, 3, @clothes_color$[@make] + "보석 주었습니다";
                    message @sd, 3, "쇠가루 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@make] + "투구"), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@make] + "보석으로 " + @clothes_color$[@make] + "투구를 만들어주었네.");
                }
            }
        }
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
    else if(@list == 5)
    {
        set @pos, list2(@make, 0, 1, "그럼.. 어떤 색깔의 호박보석을 가지고 왔는가?", "황금호박보석입니다.");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS001;
        if(@list > 0)
        {
            setarray @clothes_color$[1], "황금호박";

            set @pos, list2(@make2, 0, 0, "그래.. " + @clothes$[@make] + "보석으로 어떤 아이템을 만들텐가?", @clothes_color$[@make] + "투구입니다.");
            if(@pos == 1 && @make2 == 1)
            {
                if(countitem(name2itemid(@clothes_color$[@make] + "보석")) == 0 || countitem(name2itemid("쇠가루")) < 2)
                {
                    set @pos, dialog(0, 0, 0, "자네 혹시 재료를 모르는 것인가? " + @clothes_color$[@make] + "보석과 쇠가루 2개를 가지고 와야 " + @clothes_color$[@make] + "투구를 만들어 줄 수 있네.");
                }
                else
                {
                    delitem @sd, name2itemid(@clothes_color$[@make] + "보석"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    delitem @sd, name2itemid("쇠가루"), 1;
                    message @sd, 3, @clothes_color$[@make] + "보석 주었습니다";
                    message @sd, 3, "쇠가루 주었습니다";
                    additem @sd, name2itemid(@clothes_color$[@make] + "투구"), 1;
                    set @pos, dialog(0, 0, 1, @clothes_color$[@make] + "보석으로 " + @clothes_color$[@make] + "투구를 만들어주었네.");
                }
            }
        }
        deletearray @clothes_color$[1], getarraysize(@clothes_color$);
    }
}