{
    image 0, 498, 0;
    set @sd, mysd();
    set @id, myid();
    set @kingname$, "장안황제";
    if(get_level(@sd) < 99)
    {
        set @pos, dialog(0, 0, 0, "감히 여기가 어디라고!");
        end;
    }
    else if($quest_100_3 != 1)
    {
        if(get_level(@sd) >= 56)
        {
START0:
            set @pos, dialog (0, 0, 1, "무례하게 폐하께 직접!\ 폐하께 전할 말씀은 나를 통해서 하시오!");
            if(@pos == 0) end;
START1:
            set @pos, dialog (0, 1, 1, "폐하게 임무를 받으려고 하시는 겁니까? 임무를 완수하면 많은 경험치를 받으시겠지만, 하지 못하면 형벌을 받게 된다오!");
            if(@pos == 0) end;
            else if(@pos == -1) goto START0;

            set @list1, list2(@list, 0, 1, "그래도 임무를 받으시려오?", "네, 받겠습니다.", "무서워서 안되겠어요.");
            if(@list1 == 0) end;
            else if(@list1 == -1) goto START1;

            if(@list == 1)
            {
                if(get_joblevel(@sd) < 1)
                {
                    set @rand_quest, rand(0, 9);
                    if(@rand_quest == 0) { set $quest_100_name_2$, "녹살쾡이"; set $quest_100_exp_2, 1500000; }
                    else if(@rand_quest == 1) { set $quest_100_name_2$, "표황살쾡이"; set $quest_100_exp_2, 2200000; }
                    else if(@rand_quest == 2) { set $quest_100_name_2$, "외눈황천구"; set $quest_100_exp_2, 3000000; }
                    else if(@rand_quest == 3) { set $quest_100_name_2$, "외눈자천구"; set $quest_100_exp_2, 35000000; }
                    else if(@rand_quest == 4) { set $quest_100_name_2$, "흑혈후"; set $quest_100_exp_2, 4000000; }
                    else if(@rand_quest == 5) { set $quest_100_name_2$, "녹혈후"; set $quest_100_exp_2, 4000000; }
                    else if(@rand_quest == 6) { set $quest_100_name_2$, "흑선문후"; set $quest_100_exp_2, 5000000; }
                    else if(@rand_quest == 7) { set $quest_100_name_2$, "흡혈강시"; set $quest_100_exp_2, 5500000; }
                    else if(@rand_quest == 8) { set $quest_100_name_2$, "아기강시"; set $quest_100_exp_2, 4000000; }
                    else if(@rand_quest == 9) { set $quest_100_name_2$, "흑발강시"; set $quest_100_exp_2, 12000000; }
                }
                else if(get_joblevel(@sd) >= 1 && get_joblevel(@sd) <= 2)
                {
                    set @rand_quest, rand(0, 4);
                    if(@rand_quest == 0) { set $quest_100_name_2$, "선비평민"; set $quest_100_exp_2, 20000000; }
                    else if(@rand_quest == 1) { set $quest_100_name_2$, "선비검객"; set $quest_100_exp_2, 25000000; }
                    else if(@rand_quest == 2) { set $quest_100_name_2$, "흡혈강시"; set $quest_100_exp_2, 25000000; }
                    else if(@rand_quest == 3) { set $quest_100_name_2$, "아기강시"; set $quest_100_exp_2, 25000000; }
                    else if(@rand_quest == 4) { set $quest_100_name_2$, "흑발강시"; set $quest_100_exp_2, 25000000; }
                }
                else if(get_joblevel(@sd) >= 3)
                {
                    set @rand_quest, rand(0, 4);
                    if(@rand_quest == 0) { set $quest_100_name_2$, "선비평민"; set $quest_100_exp_2, 20000000; }
                    else if(@rand_quest == 1) { set $quest_100_name_2$, "선비검객"; set $quest_100_exp_2, 25000000; }
                    else if(@rand_quest == 2) { set $quest_100_name_2$, "흉노평민"; set $quest_100_exp_2, 90000000; }
                    else if(@rand_quest == 3) { set $quest_100_name_2$, "흉노무사"; set $quest_100_exp_2, 100000000; }
                    else if(@rand_quest == 4) { set $quest_100_name_2$, "흉노전사"; set $quest_100_exp_2, 150000000; }
                }
                set @pos, dialog (0, 0, 1, "어명이오! "+ $quest_100_name_2$ +"을 잡으라는 폐하의 말씀이 있으셨소! 임무를 수행한 후에는 시간을 지체하지 말고 바로 황궁으로 돌아오시오!");
                if(@pos == 0) end;

                set $quest_100_3, 1;
                set $quest_100_count_2, 0;
                addlegend @sd, 43, 6, 0, 17, @kingname$ + "에게 " + $quest_100_name_2$ + "을 잡는 임무를 받음";
                end;
            }
        }
    }
    else if($quest_100_3 == 1)
    {
        set @my_exp, get_exp(@sd);
        if($quest_100_count_2 == 0)
        {
            set @pos, dialog(0, 0, 1, "감히 폐하의 임무를 취소해달라고? 이놈에게 형벌을 가하라! ");
            if(@pos == 0) end;

            if(spellaethered(@sd, name2spellid("왕의저주")) == 0)
            {
                spellaether @sd, name2spellid("왕의저주"), 600;
                set_ac @sd, get_ac(@sd)+40;
                effect @id, 13;
                sound @id, 69;
            }

            dellegend @sd, 43;
            set $quest_100_3, 0; 
            set $quest_100_count_2, 0;
            set $quest_100_name_2$, "";
            set $quest_100_exp_2, 0;
            set @pos, dialog(0, 0, 0, "이 형벌로 너의 임무가 지워졌으니, 다시 임무를 받을 수 있을 것이다.");
            end;
        }
        else
        {
            set @cal_exp, @my_exp+($quest_100_exp_2*#server_exprate);
            if(@my_exp < -2147483648 && @my_exp <= -1) 
            { 
                if(@cal_exp > -1)
                {
                    set @pos, dialog(0, 0, 0, "경험치를 팔고 오게");
                    end;
                }
            }
            set @pos, dialog(0, 0, 1, "어명을 받든 공을 높이 사신 폐하께서 그대에게 경험치 " + ($quest_100_exp_2*#server_exprate) + " 을 하사하십니다.");
            if(@pos == 0) end;

            dellegend @sd, 43;
            set $quest_100_3, 0;
            set $quest_100_count_2, 0;
            set @giveexp, $quest_100_exp_2;
            giveexp @sd, @giveexp;
            end;
        }
    }
    else { end; }
}