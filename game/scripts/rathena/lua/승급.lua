{
    image 0, 50, 0;
    set @sd, mysd();
    set @id, myid();
RESET:

    set @pos, list1("안녕하세요. 어떻게 오셨나요?", "팔괘", "순수한물", "황금호박무기만들기");
    if(@pos == 0) { end; }
    else if(@pos == 1)
    {
POS00000:
        set @pos, dialog(0, 0, 1, "어서 오십시요. 저는 옥황상제의 막내딸 랑구륜이랍니다.");
        if(@pos == 0) { end; }
POS00001:
        set @pos, dialog(0, 1, 1, "저는 8개의 괘를 팔괘로 바꾸어드립니다.");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS00000; }
        set @pos, list2(@ret, 0, 1, "8개의 괘를 팔괘로 바꾸시겠어요?", "네", "아니요, 팔괘가 다 없어요...");
        if(@pos == 0) { end; }
        else if(@pos == -1) { goto POS00001; }
        if(@ret == 1)
        {
            setarray @chk_item$[0], "건괘", "곤괘", "진괘", "감괘", "리괘", "태괘", "선괘", "간괘", "-1";
            set @pos, dialog(0, 0, 1, "어디보자...");
            for(set @i, 0; @i < 8; set @i, @i+1)
            {
                if(countitem(name2itemid(@chk_item$[@i])) == 0)
                {
                    if(@i <= 2)
                    {
                        set @pos, dialog(0, 0, 1, "가장 기본적인 " + @chk_item$[@i] + "가 없으시네요... 아쉽습니다...");
                    }
                    else
                    {
                        set @pos, dialog(0, 0, 1, @chk_item$[@i] + "가 없으시네요... 아쉽습니다...");
                    }
                    deletearray @chk_item$[0], getarraysize(@chk_item$);
                    goto RESET;
                }
            }
            set @pos, dialog(0, 0, 1, "8개의 괘들을 다 가져오셨군요. 팔괘를 만들어 드리겠습니다, 잠시만 기달려주세요.");
            if(@pos == 0) end;

            set @pos, dialog(0, 0, 1, "자 팔괘를 만들어 드렸습니다. 그럼 안녕히가십시요.");
            if(@pos == 0) end;

            for(set @i, 0; @i < 8; set @i, @i+1)
            {
                delitem @sd, name2itemid(@chk_item$[@i]), 1;
            }
            for(set @i, 0; @i < 8; set @i, @i+1)
            {
                message @sd, 3, @chk_item$[@i] + " 주었습니다";
            }

            additem @sd, name2itemid("팔괘"), 1;
            deletearray @chk_item$[0], getarraysize(@chk_item$);
            end;
        }
        else if(@ret == 2)
        {
            set @pos, dialog(0, 0, 1, "다음엔 팔괘를 다 모아오세요...");
            goto RESET;
        }
    }
    else if(@pos == 2)
    {
        if($clear_shiled == 1)
        {
            set @pos, list2(@clear, 0, 0, " ", "물을 정화시키는 방법을 아시나요?");
            if(@pos == 0) end;
            else if(@clear != 1) end;

            set @pos, list2(@shiled, 0, 0, "물론 알고 있지요. 하지만 그 방법을 배우기 위해서는 대가가 필요하지요.", "무슨 대가인가요?");
            if(@pos == 0) end;
            else if(@shiled != 1) end;

            set @pos, list2(@apple, 0, 0, "신선한 사과를 먹어본지 참 오래 되었는데..가서 홍옥 3개만 가지고 오세요.", "예. 알겠습니다.", "홍옥!! 차라리 내가 먹고 말지..");
            if(@pos == 0) end;
            else if(@apple != 1) end;

            set $clear_shiled, 2;
            addlegend @sd, 23, 7, 0, 1, "랑구륜의 부탁을 들어주자.";
            set @pos, dialog(0, 0, 0, "아참 전 국광보다는 홍옥을 좋아하니 꼭 홍옥으로 3개를 가져오세요.");
            end;
        }
        if($clear_shiled == 2)
        {
            if(countitem(name2itemid("홍옥")) < 3)
            {
                set @pos, dialog(0, 0, 0, "아직 홍옥 3개를 구하시지 못하신거군요.");
                end;
            }
            else
            {
COS001:
                set @pos, dialog(0, 0, 1, "어머 홍옥을 가져오셨군요. 이건 제가 잘 먹을께요.");
                if(@pos == 0) end;

                set $clear_shiled, 3;
                delitem @sd, name2itemid("홍옥"), 3;
                message @sd, 3, "홍옥 주었습니다";
                additem @sd, name2itemid("정화비서"), 1;
                addlegend @sd, 23, 7, 0, 1, "랑구륜의 부탁을 들어주었다.";
COS002:
                set @pos, dialog(0, 0, 1, "우물우물... 아... 역시 언제 먹어도 홍옥의 맛이 최고야.");
                if(@pos == 0) end;
COS003:
                set @pos, dialog(0, 1, 0, "정화비서를 가져다 주면 될거에요.");
                if(@pos == -1) goto COS002;
                end;
            }
        }
        else
        {
            set @pos, dialog(0,0,0, "아, 홍옥의 그 광채...빛깔...맛...언제 생각해도 황홀하네.");
            end;
        }
    }
    else if(@pos == 3)
    {
        set @pos, dialog(0, 0, 1, "호박의 정수.. 황금호박별을 가지고 오셨습니까. 이것으로 무기를 만드시렵니까..");
        if(@pos == 0) { end; }
        if(countitem(name2itemid("황금호박별")) > 0)
        {
        }
        else
        {
            set @pos, dialog(0, 0, 0, "황금호박별을 가지고 있지 않으시군요. 아직은 때가 아닌가보군요..");
            if(@pos == 0) { end; }
        }
    }
}