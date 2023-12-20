{
    image 0, 50, 0;
    set @sd, mysd();
    set @id, myid();
    set @myjob, get_job(@sd);
    set @myjoblv, get_joblevel(@sd);

    switch(@myjoblv)
    {
    case 0:
        set @jobname$, "전사";
        break;
    case 1:
        set @jobname$, "검객";
        break;
    case 2:
        set @jobname$, "검제";
        break;
    case 3:
        set @jobname$, "검황";
        break;
    case 4:
        set @jobname$, "검성";
        break;
    }
RESET:
    set @pos, list2(@ret, 0, 0, "안녕하세요. 어떻게 오셨나요?", @jobname$ + "로서 승급을 원합니다", "기술을 배울래요", "3차승급 옷을 원합니다");
    if(@pos == 0) { end; }
    else if(@ret == 1)
    {
        if(@myjob == 1 && @myjoblv < 3)
        {
            setarray @hp[0], 70000, 200000, 600000, 1400000;
            setarray @mp[0], 0, 0, 0, 0;

            if(get_level(@sd) >= 99 && get_purehp(@sd) >= @hp[@myjoblv] && get_puremp(@sd) >= @mp[@myjoblv])
            {
                if(@myjoblv == 0) { setarray @needitem$[0], "팔괘"; }
                else if(@myjoblv == 1) { setarray @needitem$[0], "수룡의비늘","화룡의비늘"; }
                else if(@myjoblv == 2) { setarray @needitem$[0], "천성현의증표"; }
                else if(@myjoblv == 3) { setarray @needitem$[0],"반고의심장"; }

                for(set @i, 0; @i < getarraysize(@needitem$); set @i, @i+1)
                {
                    if(countitem(name2itemid(@needitem$[@i])) == 0)
                    {
                        set @pos, dialog(0,0,0, @needitem$[@i] + " (이)가 없습니다.");
                        deletearray @needitem$[0], getarraysize(@needitem$);
                        deletearray @hp[0], getarraysize(@hp);
                        deletearray @mp[0], getarraysize(@mp);
                        end;
                    }
                }
                if(@i == getarraysize(@needitem$))
                {
                    for(set @i, 0; @i < getarraysize(@needitem$); set @i, @i+1)
                    {
                        delitem @sd, name2itemid(@needitem$[@i]),1;
                    }

                    set_joblevel @sd, @myjoblv+1;

                    spellaether @sd, name2spellid("승급보상"), 1;

                    broadcast 4, "(( [" + get_name(@sd) + "]님이 " + get_jobname(@sd) + "으로 승급하였습니다. 축하합니다! ))";
                }
            }
            else
            {
                set @pos, dialog(0,0,0, "좀더 수련을 쌓고오세요.");
            }
            deletearray @needitem$[0], getarraysize(@needitem$);
            deletearray @hp[0], getarraysize(@hp);
            deletearray @mp[0], getarraysize(@mp);
            end;
        }
        else
        {
            set @pos, dialog(0, 0, 1, "당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.");
            end;
        }
    }
    else if(@ret == 2)
    {
        if(@myjob == 1)
        {
            set @ret, list1("안녕하세요. 어떻게 오셨나요?", "1차 승급기술", "2차 승급기술", "3차 승급기술", "4차 승급기술");
            if(@ret == 1 && @myjoblv >= 1)
            {
                setarray @magic$[0], "백호참";
                set @ret, list1("안녕하세요. 어떤 기술을 배울래요?", @magic$[0]);
                if(@ret == 1)
                {
                    set @pos, dialog(0, 0, 1, @magic$[@ret-1] + "을 배우기 위해선 5000만의 경험치가 필요합니다.");
                    if(@pos == 0) { deletearray @magic$[0], getarraysize(@magic$); end; }

                    if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                    else { set @exp1, get_exp(@sd); set @exp2, 0; }
                    
                    if((@exp1-50000000) < 0) { set @pos, dialog(0, 0, 0, "경험치가 모자랍니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                    if(checkspell(name2spellid(@magic$[@ret-1])) == 52 )
                    {
                        set_exp @sd, ((@exp1-50000000)+@exp2);
                        addspell name2spellid(@magic$[@ret-1]);
                        set @pos, dialog(0, 0, 0, @magic$[@ret-1] + "마법을 드렸습니다.");
                        updatestatus @sd, 2;
                    }
                    else { set @pos, dialog(0, 0, 0, "이미 배운 마법입니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                }
                deletearray @magic$[0], getarraysize(@magic$);
            }
            if(@ret == 2 && @myjoblv >= 2)
            {
                setarray @magic$[0], "어검술", "진백호령";
                set @ret, list1("안녕하세요. 어떤 기술을 배울래요?", @magic$[0], @magic$[1]);
                if(@ret == 0) { end; }
                set @pos, dialog(0, 0, 1, @magic$[@ret-1] + "을 배우기 위해선 5000만의 경험치가 필요합니다.");
                if(@pos == 0) { deletearray @magic$[0], getarraysize(@magic$); end; }

                if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                else { set @exp1, get_exp(@sd); set @exp2, 0; }
                    
                if((@exp1-50000000) < 0) { set @pos, dialog(0, 0, 0, "경험치가 모자랍니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                if(checkspell(name2spellid(@magic$[@ret-1])) == 52 )
                {
                    set_exp @sd, ((@exp1-50000000)+@exp2);
                    addspell name2spellid(@magic$[@ret-1]);
                    set @pos, dialog(0, 0, 0, @magic$[@ret-1] + "마법을 드렸습니다.");
                    updatestatus @sd, 2;
                }
                else { set @pos, dialog(0, 0, 0, "이미 배운 마법입니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                deletearray @magic$[0], getarraysize(@magic$);
            }
            if(@ret == 3 && @myjoblv >= 3)
            {
                setarray @magic$[0], "초혼비무", "쇄혼비무", "포효검황";
                set @ret, list1("안녕하세요. 어떤 기술을 배울래요?", @magic$[0], @magic$[1], @magic$[2]);
                if(@ret == 0) { end; }
                set @pos, dialog(0, 0, 1, @magic$[@ret-1] + "을 배우기 위해선 5000만의 경험치가 필요합니다.");
                if(@pos == 0) { deletearray @magic$[0], getarraysize(@magic$); end; }

                if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                else { set @exp1, get_exp(@sd); set @exp2, 0; }
                    
                if((@exp1-50000000) < 0) { set @pos, dialog(0, 0, 0, "경험치가 모자랍니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                if(checkspell(name2spellid(@magic$[@ret-1])) == 52 )
                {
                    set_exp @sd, ((@exp1-50000000)+@exp2);
                    addspell name2spellid(@magic$[@ret-1]);
                    set @pos, dialog(0, 0, 0, @magic$[@ret-1] + "마법을 드렸습니다.");
                    updatestatus @sd, 2;
                }
                else { set @pos, dialog(0, 0, 0, "이미 배운 마법입니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                deletearray @magic$[0], getarraysize(@magic$);
            }
            if(@ret == 4 && @myjoblv >= 4)
            {
                setarray @magic$[0], "혈겁만파", "극'백호참", "운공체식";
                set @ret, list1("안녕하세요. 어떤 기술을 배울래요?", @magic$[0], @magic$[1], @magic$[2]);
                if(@ret == 0) { end; }
                set @pos, dialog(0, 0, 1, @magic$[@ret-1] + "을 배우기 위해선 5000만의 경험치가 필요합니다.");
                if(@pos == 0) { deletearray @magic$[0], getarraysize(@magic$); end; }

                if(get_exp(@sd) < 0) { set @exp1, 2147483647; set @exp2, (get_exp(@sd)+@exp1); }
                else { set @exp1, get_exp(@sd); set @exp2, 0; }
                    
                if((@exp1-50000000) < 0) { set @pos, dialog(0, 0, 0, "경험치가 모자랍니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                if(checkspell(name2spellid(@magic$[@ret-1])) == 52 )
                {
                    set_exp @sd, ((@exp1-50000000)+@exp2);
                    addspell name2spellid(@magic$[@ret-1]);
                    set @pos, dialog(0, 0, 0, @magic$[@ret-1] + "마법을 드렸습니다.");
                    updatestatus @sd, 2;
                }
                else { set @pos, dialog(0, 0, 0, "이미 배운 마법입니다."); deletearray @magic$[0], getarraysize(@magic$); end; }
                deletearray @magic$[0], getarraysize(@magic$);
            }
            else
            {
                goto RESET;
            }
        }
    }
    else if(@ret == 3)
    {
        if(@myjoblv < 3)
        {
            set @pos, dialog(0, 0, 1, "3차 승급을 한 뒤에 다시 오세요.");
            if(@pos == 0) { end; }
        }
    }
}