{
    set @sd, mysd();
    set @id, myid();

    set @ret, list1("안녕하세요. 어떻게 오셨나요?", "방송쿠폰", "웨딩드레스 수선");
    if(@ret == 1) goto COS001;
    else if(@ret == 2) goto COS003;
    else end;
    end;
COS001:
    set @pos, dialog(0, 0, 1, "방송쿠폰은 나만이 팔고 있지. 호호홋...");
    if(@pos == 0) end;
COS002:
    set @pos, list2(@list, 0, 0, "방송쿠폰의 가격은 100 전 이라네. 방송쿠폰을 살텐가?", "예. 구입하고 싶습니다.", "아니오. 필요 없습니다.");
    if(@pos == 0) end;
    else if(@list == 1)
    {
        if(get_level(@sd) < 21)
        {
            set @pos, dialog(0, 0, 0, "21레벨 구매가 가능합니다.");
            end;
        }
        if(#broadcast_card == 0)
        {
            if(get_money(@sd) < 100)
            {
                set @pos, dialog(0, 0, 0, "방송쿠폰의 가격은 100 전 이라네. 돈이 부족한 것 아닌가?");
                end;
            }
            else
            {
                set #broadcast_card, 1;
                set #broadcast_card_count, 0;
                additem @sd, name2itemid("방송쿠폰"), 1;
                set_money @sd, get_money(@sd)-100;
                updatestatus @sd, 2;
                set @pos, dialog(0, 0, 1, "운이 좋군. 잘 쓰시게...");
                end;
            }
        }
        else
        {
            set @pos, dialog(0, 0, 0, "이미 다 떨어졌는데...");
            end;
        }
    }
    else end;
    end;
COS003:
    set @pos, dialog(0, 0, 1, "웨딩드레스를 수선하고 싶다고? 그렇다면 웨딩드레스1,2,3,4,5,6,7 과 말린지네 7개를 가져온다면 수선해주도록 하지.");
    if(@pos == 0) end;

    set @list, list1("웨딩드레스를 수선하겠는가?", "예", "아뇨");
    if(@list == 1)
    {
        if(countitem(name2itemid("말린지네")) < 7)
        {
            set @pos, dialog(0, 0, 0, "재료가 부족한데? 재료를 다 가져와야 수선해줄 수 있어.");
            end;
        }
        else
        {
            setarray @item$[0], "웨딩드레스1", "웨딩드레스2", "웨딩드레스3", "웨딩드레스4", "웨딩드레스5", "웨딩드레스6", "웨딩드레스7";
            for(set @loop, 0; @loop < 7; set @loop, @loop+1)
            {
                if(countitem(name2itemid(@item$[@loop])) == 0)
                {
                    deletearray @item$[0], getarraysize(@item$);
                    set @pos, dialog(0, 0, 0, "재료가 부족한데? 재료를 다 가져와야 수선해줄 수 있어.");
                    end;
                }
            }
            delitem @sd, name2itemid("말린지네"), 7;
            message @sd, 3, "말린지네 주었습니다";
            for(set @loop, 0; @loop < 7; set @loop, @loop+1)
            {
                delitem @sd, name2itemid(@item$[@loop]), 1;
            }
            for(set @loop, 0; @loop < 7; set @loop, @loop+1)
            {
                message @sd, 3, @item$[@loop] + " 주었습니다";
            }
            set @rand, rand(1, 4);
            if(@rand == 1) { set @add$, "웨딩드레스8"; }
            else if(@rand == 2) { set @add$, "웨딩드레스9"; }
            else if(@rand == 3) { set @add$, "웨딩드레스10"; }
            else { set @add$, "웨딩드레스11"; }
            additem @sd, name2itemid(@add$), 1;
            deletearray @item$[0], getarraysize(@item$);
            set @pos, dialog(0, 0, 1, "여기 수선해 줬으니 잘 입으시게.");
        }
    }
    deletearray @item$[0], getarraysize(@item$);
}