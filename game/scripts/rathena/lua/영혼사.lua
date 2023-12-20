{
    set @sd, mysd();
    set @id, myid();
COS001:
    image 0, 74, 0;

    for(set @i, 0; @i < 8; set @i, @i+1)
    {
        if(get_equipitem(@sd, @i) != 0) { set @pos, dialog(0, 0, 0, "아이템을 하나라도 착용하고 있으면 날 만날수 없네!"); end; }
    }

    set @ret, list2(@list, 0, 0, "자네는 여기에 온 이유가 무엇인가?", "경험치변환", "성전환");
    if(@ret == 0) end;
    if(@list == 1)
    {
        if(get_level(@sd) < 90) { set @pos, dialog(0, 0, 1, "수련을 더 쌓으시게."); end; }
        else
        {
            set @pos, list2(@list, 1, 1, "바라는 것이 무엇인가?", "체력 사기", "마력 사기", "힘 사기", "민첩 사기", "지력 사기");
            if(@pos == 0) { end; }
            else if(@pos== -1) { goto COS001; }
            if(@list == 1)
            {
                if(get_job(@sd) == 1 || get_job(@sd) == 2)
                {
                    if(get_joblevel(@sd) == 0 && get_purehp(@sd) >= 80000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 1 && get_purehp(@sd) >= 250000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 2 && get_purehp(@sd) >= 700000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 3 && get_purehp(@sd) >= 1600000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_purehp(@sd) < 800000) { set @minus_exp_m$, "1천만"; set @minus_exp, 10000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 800000 && get_purehp(@sd) < 1000000) { set @minus_exp_m$, "2천만"; set @minus_exp, 20000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1000000 && get_purehp(@sd) < 1200000) { set @minus_exp_m$, "5천만"; set @minus_exp, 50000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1200000 && get_purehp(@sd) < 1500000) { set @minus_exp_m$, "1억"; set @minus_exp, 100000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1500000 && get_purehp(@sd) < 1700000) { set @minus_exp_m$, "2억"; set @minus_exp, 200000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1700000 && get_purehp(@sd) < 2000000) { set @minus_exp_m$, "3억"; set @minus_exp, 300000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 2000000 && get_purehp(@sd) < 2200000) { set @minus_exp_m$, "4억"; set @minus_exp, 400000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 2200000 && get_purehp(@sd) < 2500000) { set @minus_exp_m$, "6억"; set @minus_exp, 600000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 2500000 && get_purehp(@sd) < 3000000) { set @minus_exp_m$, "9억"; set @minus_exp, 900000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 3000000 && get_purehp(@sd) < 3100000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 3100000 && get_purehp(@sd) < 3200000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 25; }
                    else if(get_purehp(@sd) >= 3200000 && get_purehp(@sd) < 3300000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 10; }
                    else if(get_purehp(@sd) >= 3300000 && get_purehp(@sd) < 3400000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 5; }
                    else if(get_purehp(@sd) >= 3400000 && get_purehp(@sd) < 3500000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 3; }
                    else if(get_purehp(@sd) >= 3500000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 1; }
                }
                else if(get_job(@sd) == 3 || get_job(@sd) == 4)
                {
                    if(get_joblevel(@sd) == 0 && get_purehp(@sd) >= 35000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 1 && get_purehp(@sd) >= 170000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 2 && get_purehp(@sd) >= 550000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_joblevel(@sd) == 3 && get_purehp(@sd) >= 1000000) { set @pos, dialog(0, 0, 0, "아직도 승급을 하지 않았다니. 일단 승급을 한 뒤에 다시 오도록!"); end; }
                    if(get_purehp(@sd) < 400000) { set @minus_exp_m$, "1천만"; set @minus_exp, 10000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 400000 && get_purehp(@sd) < 500000) { set @minus_exp_m$, "2천만"; set @minus_exp, 20000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 500000 && get_purehp(@sd) < 600000) { set @minus_exp_m$, "5천만"; set @minus_exp, 50000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 600000 && get_purehp(@sd) < 700000) { set @minus_exp_m$, "1억"; set @minus_exp, 100000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 700000 && get_purehp(@sd) < 800000) { set @minus_exp_m$, "2억"; set @minus_exp, 200000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 800000 && get_purehp(@sd) < 900000) { set @minus_exp_m$, "3억"; set @minus_exp, 300000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 900000 && get_purehp(@sd) < 1000000) { set @minus_exp_m$, "4억"; set @minus_exp, 400000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1000000 && get_purehp(@sd) < 1100000) { set @minus_exp_m$, "6억"; set @minus_exp, 600000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1100000 && get_purehp(@sd) < 1200000) { set @minus_exp_m$, "9억"; set @minus_exp, 900000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1200000 && get_purehp(@sd) < 1250000) { set @minus_exp_m$, "12억"; set @minus_exp, 1200000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1250000 && get_purehp(@sd) < 1300000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 50; }
                    else if(get_purehp(@sd) >= 1300000 && get_purehp(@sd) < 1400000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 25; }
                    else if(get_purehp(@sd) >= 1400000 && get_purehp(@sd) < 1500000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 10; }
                    else if(get_purehp(@sd) >= 1500000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_vita, 5; }
                }
                if(get_exp(@sd) >= 0 && get_exp(@sd) < @minus_exp) { set @pos, dialog(0, 0, 1, "경험치가 부족하네."); end; }

                if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                else { set @exp1, get_exp(@sd); set @exp2, 0; }

                set @max_step, @exp1/@minus_exp + @exp2/@minus_exp;
                if(@minus_exp_m$ == "1천만" && @max_step > 200) { set @max_step, 200; }
                else if(@minus_exp_m$ == "2천만" && @max_step > 100) { set @max_step, 100; }
                else if(@minus_exp_m$ == "5천만" && @max_step > 40) { set @max_step, 40; }
                else if(@minus_exp_m$ == "1억" && @max_step > 20) { set @max_step, 20; }
                else if(@minus_exp_m$ == "2억" && @max_step > 10) { set @max_step, 10; }
                else if(@minus_exp_m$ == "4억" && @max_step > 5) { set @max_step, 5; }
COS002:
                set @pos, dialog(0, 0, 1, "자네 수준이면 경험치 " + @minus_exp_m$ + "에 체력 " + @plus_vita + "을 올릴 수 있겠군. 한번에 20억 이상은 팔지 못한다네.");
                if(@pos == 0) { end; }

                set @pos, input2(@input_step, 1, 6, "그래, 몇 단계를 올리겠는가? 최대 " + @max_step + "단계까지 올릴 수 있네.", " ", " ");
                if(@pos == 0) { end; }
                else if(@pos == -1) { goto COS002; }

                if(@input_step > @max_step || @input_step <= 0) 
                {
                    set @pos, dialog(0, 0, 0, "다시 한번 장난을 친다면 체력을 100으로 만들어버리겠네.");
                    end; 
                }
                else
                {
                    set_exp @sd, ((@exp1+@exp2)-(@input_step*@minus_exp));
                    set_purehp @sd, get_purehp(@sd)+(@input_step*@plus_vita);
                    set @pos, dialog(0, 0, 1, "능력치를 정상적으로 올려주었네.");
                    end;
                }
            }
            else if(@list == 2)
            {
                if(get_job(@sd) == 1 || get_job(@sd) == 2)
                {
                    if(get_puremp(@sd) < 100000) { set @minus_exp_m$, "1천만"; set @minus_exp, 10000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 100000 && get_puremp(@sd) < 150000) { set @minus_exp_m$, "2천만"; set @minus_exp, 40000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 150000 && get_puremp(@sd) < 200000) { set @minus_exp_m$, "5천만"; set @minus_exp, 50000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 200000 && get_puremp(@sd) < 250000) { set @minus_exp_m$, "1억"; set @minus_exp, 100000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 250000 && get_puremp(@sd) < 300000) { set @minus_exp_m$, "2억"; set @minus_exp, 200000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 300000 && get_puremp(@sd) < 400000) { set @minus_exp_m$, "2억"; set @minus_exp, 200000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 400000 && get_puremp(@sd) < 450000) { set @minus_exp_m$, "4억"; set @minus_exp, 400000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 450000 && get_puremp(@sd) < 500000) { set @minus_exp_m$, "6억"; set @minus_exp, 600000000; set @plus_mana,  25; }
                    else if(get_puremp(@sd) >= 500000 && get_puremp(@sd) < 550000) { set @minus_exp_m$, "10억"; set @minus_exp, 1000000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 550000 && get_puremp(@sd) < 600000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 600000 && get_puremp(@sd) < 650000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 15; }
                    else if(get_puremp(@sd) >= 650000 && get_puremp(@sd) < 700000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 10; }
                    else if(get_puremp(@sd) >= 700000 ) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 5; }
                }
                else if(get_job(@sd) == 3 || get_job(@sd) == 4)
                {
                    if(get_joblevel(@sd) == 0 && get_puremp(@sd) >= 30000) { set @pos, dialog(0, 0, 0, "더 이상 올릴 수 없습니다."); end; }
                    if(get_joblevel(@sd) == 1 && get_puremp(@sd) >= 110000) { set @pos, dialog(0, 0, 0, "더 이상 올릴 수 없습니다."); end; }
                    if(get_joblevel(@sd) == 2 && get_puremp(@sd) >= 550000) { set @pos, dialog(0, 0, 0, "더 이상 올릴 수 없습니다."); end; }
                    if(get_joblevel(@sd) == 3 && get_puremp(@sd) >= 1000000) { set @pos, dialog(0, 0, 0, "더 이상 올릴 수 없습니다."); end; }
                    if(get_puremp(@sd) < 600000) { set @minus_exp_m$, "1천만"; set @minus_exp, 10000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 600000 && get_puremp(@sd) < 700000) { set @minus_exp_m$, "2천만"; set @minus_exp, 20000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 700000 && get_puremp(@sd) < 800000) { set @minus_exp_m$, "5천만"; set @minus_exp, 50000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 800000 && get_puremp(@sd) < 900000) { set @minus_exp_m$, "1억"; set @minus_exp, 100000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 900000 && get_puremp(@sd) < 1000000) { set @minus_exp_m$, "2억"; set @minus_exp, 200000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1000000 && get_puremp(@sd) < 1100000) { set @minus_exp_m$, "3억"; set @minus_exp, 300000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1100000 && get_puremp(@sd) < 1200000) { set @minus_exp_m$, "4억"; set @minus_exp, 400000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1200000 && get_puremp(@sd) < 1400000) { set @minus_exp_m$, "6억"; set @minus_exp, 600000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1400000 && get_puremp(@sd) < 1550000) { set @minus_exp_m$, "9억"; set @minus_exp, 900000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1550000 && get_puremp(@sd) < 1600000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 25; }
                    else if(get_puremp(@sd) >= 1600000 && get_puremp(@sd) < 1650000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 15; }
                    else if(get_puremp(@sd) >= 1650000 && get_puremp(@sd) < 1700000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 10; }
                    else if(get_puremp(@sd) >= 1700000 && get_puremp(@sd) < 1750000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 5; }
                    else if(get_puremp(@sd) >= 1750000 && get_puremp(@sd) < 1800000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 3; }
                    else if(get_puremp(@sd) >= 1800000) { set @minus_exp_m$, "20억"; set @minus_exp, 2000000000; set @plus_mana, 1; }
                }
                if(get_exp(@sd) >= 0 && get_exp(@sd) < @minus_exp) { set @pos, dialog(0, 0, 1, "경험치가 부족하네."); end; }

                if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                else { set @exp1, get_exp(@sd); set @exp2, 0; }

                set @max_step, @exp1/@minus_exp + @exp2/@minus_exp;
                if(@minus_exp_m$ == "2천만" && @max_step > 100) { set @max_step, 100; }
                else if(@minus_exp_m$ == "4천만" && @max_step > 50) { set @max_step, 50; }
                else if(@minus_exp_m$ == "1억" && @max_step > 20) { set @max_step, 20; }
                else if(@minus_exp_m$ == "2억" && @max_step > 10) { set @max_step, 10; }
                else if(@minus_exp_m$ == "4억" && @max_step > 5) { set @max_step, 5; }
COS003:
                set @pos, dialog(0, 0, 1, "자네 수준이면 경험치 " + @minus_exp_m$ + "에 마력 " + @plus_mana + "을 올릴 수 있겠군. 한번에 20억 이상은 팔지 못한다네.");
                if(@pos == 0) { end; }

                set @pos, input2(@input_step, 1, 6, "그래, 몇 단계를 올리겠는가? 최대 " + @max_step + "단계까지 올릴 수 있네.", " ", " ");
                if(@pos == 0) { end; }
                else if(@pos == -1) { goto COS003; }

                if(@input_step > @max_step || @input_step <= 0) 
                {
                    set @pos, dialog(0, 0, 0, "다시 한번 장난을 친다면 마력을 100으로 만들어버리겠네.");
                    end; 
                }
                else
                {
                    set_exp @sd, ((@exp1+@exp2)-(@input_step*@minus_exp));
                    set_puremp @sd, get_puremp(@sd)+(@input_step*@plus_mana);
                    set @pos, dialog(0, 0, 1, "능력치를 정상적으로 올려주었네.");
                    end;
                }
            }
            else if(@list >= 3 && @list <= 5)    
            {
                if(get_level(@sd) < 99) { set @pos, dialog(0, 0, 0, "99레벨 이상만 가능하네."); end; }
                else
                {
                    if(get_exp(@sd) >= 0 && get_exp(@sd) < 10000000) { set @pos, dialog(0, 0, 0, "경험치가 부족하네."); end; }
                    else
                    {
                        if(@list == 3)
                        {
                            if(get_might(@sd)+1 <= 130)
                            {
                                set @message$, "힘";
                                set_might @sd, get_might(@sd)+1;
                                goto LABEL;
                            }
                            else
                            {
                                set @pos, dialog(0, 0, 0, "더 이상 능력치를 올리실 수 없습니다.");
                            }
                        }
                        else if(@list == 4)
                        {
                            if(get_will(@sd)+1 <= 130)
                            {
                                set @message$, "민첩성";
                                set_will @sd, get_will(@sd)+1;
                                goto LABEL;
                            }
                            else
                            {
                                set @pos, dialog(0, 0, 0, "더 이상 능력치를 올리실 수 없습니다.");
                            }
                        }
                        else if(@list == 5)
                        {
                            if(get_grace(@sd)+1 <= 130)
                            {
                                set @message$, "지력";
                                set_grace @sd, get_grace(@sd)+1;
                                goto LABEL;
                            }
                            else
                            {
                                set @pos, dialog(0, 0, 0, "더 이상 능력치를 올리실 수 없습니다.");
                            }
                        }
                        end;
                    LABEL:
                        set_exp @sd, get_exp(@sd)-10000000;
                        updatestatus @sd, 0;
                        set @pos, dialog(0, 0, 1, "정상적으로 " + @message$ + " 1 올려주었네.");
                    }
                }
            }
            else end;
        }
    }
    else if(@list == 2)
    {
        if(get_level(@sd) < 50) { set @pos, dialog(0, 0, 0, "50레벨 이상만 가능하네."); end; }
        else
        {
            if($marry_check != 0) { set @pos, dialog(0, 0, 0, "결혼한 사람은 성전환이 불가능합니다."); end; }

            if(get_gender(@sd) == 1) { set @gender$, "남자"; }
            else { set @gender$, "여자"; }

            set @list2, list1("정말로 " + @gender$ + "가 되고 싶은가? 기혼이거나 아이템을 착용한 상태에선 불가능하고 10만전의 시술비가 필요하다네. 그리고, 성별이 바뀐 후에 어울리는 머리모양으로 바꿀 기회를 줄 터이니 놓치지 않도록 하게. 준비 되었는가?", "예", "아니오");
            if(@list2 == 1)
            {
                if(get_money(@sd) < 100000) { set @pos, dialog(0, 0, 0, "금전이 부족하다네!"); end; }

                if(get_gender(@sd) == 0) { set_gender @sd, 1; }
                else if(get_gender(@sd) == 1) { set_gender @sd, 0; }

                set_money @sd, get_money(@sd)-100000;
                updatestatus @sd, 2;
REHAIR1:
                set @face, 1;
                set @haircolor, get_haircolor(@sd);
HAIR1:
                set @pos, list3(@ret, 0, 0, @face, @haircolor, "이 모습으로 바꾸시겠습니까?\\n\\n비용은 1만전 입니다.", "이 모습을 선택하겠습니다.", "다른 머리모양을 보여주세요.", "바꾸지 않겠습니다.");
                if(@pos == 0) { end; }
                else if(@ret == 1)
                {
                    set @pos, dialog(0, 0, 0, "자! 해주었다네, 다음에 또오시게.");
                    set_face @sd, @face;
                    updatexy @sd;
                    updatelookpc @sd, 0;
                    updateallpc @sd;
                }
                else if(@ret == 2)
                {
                    set @face, @face+1;
                    if(@face < 30) { goto HAIR1; }
                    else
                    {
                        set @temp, list1("방금 보신 것이 마지막입니다.\\n\\n처음부터 다시 보시겠습니까?", "예");
                        if(@temp == 1) { goto REHAIR1; }
                        end;
                    }
                }
                else end;
            }
            else if(@list2 == 2)
            {
                set @pos, dialog(0,0,0, "다시 한번 생각해보게.");
            }
        }
    }
}