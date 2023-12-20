{
    image 0, 497, 4;
    set @sd, mysd();
    set @id, myid();
    set @kingname$, "부여왕";
    if(get_level(@sd) < 56)
    {
        set @pos, dialog(0, 0, 0, "감히 여기가 어디라고!");
        end;
    }
    else if(get_nation(@sd) != 2)
    {
        set @pos, dialog(0, 0, 0, "여기는 부여왕이 계시는 곳입니다.");
        end;
    }
    else if($quest_100_1 != 1)
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
                if(get_level(@sd) >= 56 && get_level(@sd) < 70)
                {
                    set @rand_quest, rand(0, 7);
                    if(@rand_quest == 0) { set $quest_100_name$, "자호"; set $quest_100_exp, 250000; }
                    else if(@rand_quest == 1) { set $quest_100_name$, "친자호"; set $quest_100_exp, 350000; }
                    else if(@rand_quest == 2) { set $quest_100_name$, "구자호"; set $quest_100_exp, 380000; }
                    else if(@rand_quest == 3) { set $quest_100_name$, "가재"; set $quest_100_exp, 550000; }
                    else if(@rand_quest == 4) { set $quest_100_name$, "전갈"; set $quest_100_exp, 850000; }
                    else if(@rand_quest == 5) { set $quest_100_name$, "전갈장"; set $quest_100_exp, 950000; }
                    else if(@rand_quest == 6) { set $quest_100_name$, "가재장"; set $quest_100_exp, 600000; }
                    else if(@rand_quest == 7) { set $quest_100_name$, "서현가재"; set $quest_100_exp, 3000000; }
                    
                }
                else if(get_level(@sd) >= 70 && get_level(@sd) < 90)
                {
                    set @rand_quest, rand(0, 6);
                    if(@rand_quest == 0) { set $quest_100_name$, "유령"; set $quest_100_exp, 1300000; }
                    else if(@rand_quest == 1) { set $quest_100_name$, "초급유령"; set $quest_100_exp, 1480000; }
                    else if(@rand_quest == 2) { set $quest_100_name$, "중급유령"; set $quest_100_exp, 1300000; }
                    else if(@rand_quest == 3) { set $quest_100_name$, "고급유령"; set $quest_100_exp, 1660000; }
                    else if(@rand_quest == 4) { set $quest_100_name$, "자령"; set $quest_100_exp, 2000000; }
                    else if(@rand_quest == 5) { set $quest_100_name$, "주령"; set $quest_100_exp, 2000000; }
                    else if(@rand_quest == 6) { set $quest_100_name$, "지령"; set $quest_100_exp, 2000000; }
                }
                else if(get_level(@sd) >= 90)
                {
                    set @rand_quest, rand(0,2);
                    if(@rand_quest == 0) { set $quest_100_name$, "처녀귀신"; set $quest_100_exp, 4500000; }
                    else if(@rand_quest == 1) { set $quest_100_name$, "달걀귀신"; set $quest_100_exp, 4500000; }
                    else if(@rand_quest == 2) { set $quest_100_name$, "불귀신"; set $quest_100_exp, 5000000; }
                }
                set @pos, dialog (0, 0, 1, "어명이오! "+ $quest_100_name$ +"을 잡으라는 폐하의 말씀이 있으셨소! 임무를 수행한 후에는 시간을 지체하지 말고 바로 황궁으로 돌아오시오!");
                if(@pos == 0) end;

                set $quest_100_1, 1;
                set $quest_100_count, 0;
                addlegend @sd, 8, 6, 0, 17, @kingname$ + "에게 " + $quest_100_name$ + "을 잡는 임무를 받음";
                end;
            }
        }
    }
    else if($quest_100_1 == 1)
    {
        set @my_exp, get_exp(@sd);
        if($quest_100_count == 0)
        {
            set @pos, dialog(0, 0, 1, "감히 폐하의 임무를 취소해달라고? 이놈에게 형벌을 가하라! ");
            if(@pos == 0) end;

            if(spellaethered(@sd, name2spellid("왕의저주")) == 0)
            {
                spellaether @sd, name2spellid("왕의저주"), 430;
                set_ac @sd, get_ac(@sd)+40;
                effect @id, 13;
                sound @id, 69;
            }

            dellegend @sd, 8;
            set $quest_100_1, 0; 
            set $quest_100_count, 0;
            set $quest_100_name$, "";
            set $quest_100_exp, 0;
            set @pos, dialog(0, 0, 0, "이 형벌로 너의 임무가 지워졌으니, 다시 임무를 받을 수 있을 것이다.");
            end;
        }
        else
        {
            set @cal_exp, @my_exp+($quest_100_exp*#server_exprate);
            if(@my_exp < -2147483648 && @my_exp <= -1) 
            { 
                if(@cal_exp > -1)
                {
                    set @pos, dialog(0, 0, 0, "경험치를 팔고 오게");
                    end;
                }
            }
            set @pos, dialog(0, 0, 1, "어명을 받든 공을 높이 사신 폐하께서 그대에게 경험치 " + ($quest_100_exp*#server_exprate) + " 을 하사하십니다.");
            if(@pos == 0) end;

            dellegend @sd, 8;
            set $quest_100_1, 0;
            set $quest_100_count, 0;
            set @giveexp, $quest_100_exp;
            giveexp @sd, @giveexp;
            end;
        }
    }
    else { end; }
}